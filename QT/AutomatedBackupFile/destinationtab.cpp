#include "destinationtab.h"
#include "ui_destinationtab.h"
#include "cloudprovider.h"
#include "cloudauthdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QDebug>

DestinationTab::DestinationTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DestinationTab)
    , m_destinationManager(new DestinationManager(this))
    , m_backupFileMonitor(new BackupFileMonitor(this))
{
    ui->setupUi(this);
    setupConnections();
    setupFileMonitorConnections();
    
    // Initialize retention policy UI
    ui->spinRetentionDays->setValue(m_destinationManager->getRetentionPolicy().getRetentionDays());
    ui->chkAutoCleanup->setChecked(m_destinationManager->getRetentionPolicy().isAutoCleanupEnabled());
    
    // Setup table
    ui->tableDestinations->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableDestinations->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableDestinations->horizontalHeader()->setStretchLastSection(true);
    
    // Load saved destinations and file monitor state
    m_destinationManager->loadFromFile("destinations.json");
    m_backupFileMonitor->loadState("file_monitor.json");
    
    // Add existing destinations to file monitor
    QList<BackupDestination*> destinations = m_destinationManager->getAllDestinations();
    for (BackupDestination *dest : destinations) {
        if (dest->getType() == DestinationType::Local || dest->getType() == DestinationType::Network) {
            m_backupFileMonitor->addDestinationPath(dest->getId(), dest->getPath());
        }
    }
    
    refreshDestinationTable();
    updateMonitoringStatus();
}

DestinationTab::~DestinationTab()
{
    // Save destinations and file monitor state before destroying
    m_destinationManager->saveToFile("destinations.json");
    m_backupFileMonitor->saveState("file_monitor.json");
    delete ui;
}

void DestinationTab::setupConnections()
{
    // UI connections
    connect(ui->btnBrowseDestination, &QPushButton::clicked, this, &DestinationTab::onAddLocalDestination);
    connect(ui->btnAddCloudDest, &QPushButton::clicked, this, &DestinationTab::onAddCloudDestination);
    connect(ui->btnRemoveDestination, &QPushButton::clicked, this, &DestinationTab::onRemoveDestination);
    connect(ui->spinRetentionDays, QOverload<int>::of(&QSpinBox::valueChanged), this, &DestinationTab::onRetentionDaysChanged);
    connect(ui->chkAutoCleanup, &QCheckBox::toggled, this, &DestinationTab::onAutoCleanupToggled);
    
    // Connect new monitoring UI controls
    connect(ui->chkEnableDestMonitoring, &QCheckBox::toggled,
            this, &DestinationTab::onToggleMonitoring);
    connect(ui->btnViewDestChanges, &QPushButton::clicked,
            this, &DestinationTab::onViewChangeHistory);
    connect(ui->btnScanDestination, &QPushButton::clicked,
            this, [this]() { m_backupFileMonitor->scanAllDestinations(); });
    
    // Manager connections
    connect(m_destinationManager, &DestinationManager::destinationAdded, this, &DestinationTab::onDestinationAdded);
    connect(m_destinationManager, &DestinationManager::destinationRemoved, this, &DestinationTab::onDestinationRemoved);
    connect(m_destinationManager, &DestinationManager::destinationUpdated, this, &DestinationTab::onDestinationUpdated);
    connect(m_destinationManager, &DestinationManager::destinationStatusChanged, this, &DestinationTab::onDestinationStatusChanged);
    connect(m_destinationManager, &DestinationManager::checkCompleted, this, &DestinationTab::onCheckCompleted);
    connect(m_destinationManager, &DestinationManager::error, this, &DestinationTab::onError);
}

void DestinationTab::setupFileMonitorConnections()
{
    // Connect file monitor signals
    connect(m_backupFileMonitor, &BackupFileMonitor::fileAdded,
            this, &DestinationTab::onFileAdded);
    connect(m_backupFileMonitor, &BackupFileMonitor::fileModified,
            this, &DestinationTab::onFileModified);
    connect(m_backupFileMonitor, &BackupFileMonitor::fileDeleted,
            this, &DestinationTab::onFileDeleted);
    connect(m_backupFileMonitor, &BackupFileMonitor::scanCompleted,
            this, &DestinationTab::onScanCompleted);
    connect(m_backupFileMonitor, &BackupFileMonitor::changeDetected,
            this, &DestinationTab::onChangeDetected);
    connect(m_backupFileMonitor, &BackupFileMonitor::monitoringStateChanged,
            this, &DestinationTab::onMonitoringStateChanged);
}

void DestinationTab::onAddLocalDestination()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Backup Destination"),
                                                      QDir::homePath(),
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (path.isEmpty()) {
        return;
    }
    
    auto *destination = new BackupDestination(path, DestinationType::Local);
    
    if (m_destinationManager->addDestination(destination)) {
        // Add to file monitor
        m_backupFileMonitor->addDestinationPath(destination->getId(), path);
        QMessageBox::information(this, tr("Success"), tr("Destination added successfully"));
    } else {
        delete destination;
    }
}

void DestinationTab::onAddCloudDestination()
{
    // Get available cloud providers
    QStringList cloudProviders = m_destinationManager->getAvailableCloudProviders();
    
    bool ok;
    QString providerName = QInputDialog::getItem(this, tr("Cloud Provider"),
                                                 tr("Select cloud provider:"),
                                                 cloudProviders, 0, false, &ok);
    
    if (!ok || providerName.isEmpty()) {
        return;
    }
    
    // Check if provider is fully implemented
    if (providerName == "OneDrive" || providerName == "Amazon S3") {
        QMessageBox::information(this, tr("Not Implemented"), 
                                tr("%1 support is not fully implemented yet.\n\n"
                                   "You can use Test Mode to simulate cloud storage.").arg(providerName));
        return;
    }
    
    // Show authentication dialog with instructions
    CloudAuthDialog authDialog(providerName, this);
    if (authDialog.exec() != QDialog::Accepted) {
        return;
    }
    
    QMap<QString, QString> credentials = authDialog.getCredentials();
    
    // Create cloud provider instance
    CloudProvider *provider = nullptr;
    
    if (authDialog.isTestModeEnabled()) {
        // Use mock provider for testing
        provider = new MockCloudProvider(this);
    } else {
        // Create real provider
        provider = CloudProviderFactory::createProvider(providerName, this);
        if (!provider) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create cloud provider"));
            return;
        }
        
        // Check if credentials are provided
        if (credentials.value("access_token").isEmpty() && 
            credentials.value("client_id").isEmpty()) {
            QMessageBox::warning(this, tr("Missing Credentials"), 
                                tr("Please provide authentication credentials or enable Test Mode"));
            delete provider;
            return;
        }
    }
    
    // Authenticate with the provider
    if (!provider->authenticate(credentials)) {
        QMessageBox::critical(this, tr("Authentication Failed"), 
                             tr("Failed to authenticate with %1: %2")
                             .arg(providerName, provider->getLastError()));
        delete provider;
        return;
    }
    
    // Test connection
    QMessageBox *progressMsg = new QMessageBox(this);
    progressMsg->setWindowTitle(tr("Connecting"));
    progressMsg->setText(tr("Testing connection to %1...").arg(provider->getProviderName()));
    progressMsg->setStandardButtons(QMessageBox::NoButton);
    progressMsg->show();
    QApplication::processEvents();
    
    bool connected = provider->testConnection();
    progressMsg->close();
    delete progressMsg;
    
    if (!connected) {
        QMessageBox::critical(this, tr("Connection Failed"), 
                             tr("Failed to connect to %1: %2")
                             .arg(provider->getProviderName(), provider->getLastError()));
        delete provider;
        return;
    }
    
    // Get a name/path for this destination
    QString defaultName = authDialog.isTestModeEnabled() ? 
                         provider->getProviderName() : providerName;
    QString name = QInputDialog::getText(this, tr("Destination Name"),
                                        tr("Enter a name for this destination:"),
                                        QLineEdit::Normal, defaultName, &ok);
    
    if (!ok || name.isEmpty()) {
        delete provider;
        return;
    }
    
    // Create destination
    auto *destination = new BackupDestination(name, DestinationType::Cloud);
    if (authDialog.isTestModeEnabled()) {
        destination->setUsername("[Test Mode]");
    } else {
        destination->setUsername(credentials.value("access_token").left(20) + "..."); // Store masked token
    }
    
    if (m_destinationManager->addDestination(destination)) {
        // Associate the cloud provider with this destination
        m_destinationManager->setCloudProvider(destination->getId(), provider);
        
        // Show space information
        qint64 available = provider->getAvailableSpace();
        qint64 total = provider->getTotalSpace();
        
        // Format bytes helper
        auto formatBytes = [](qint64 bytes) -> QString {
            const qint64 KB = 1024;
            const qint64 MB = KB * 1024;
            const qint64 GB = MB * 1024;
            const qint64 TB = GB * 1024;
            
            if (bytes >= TB) return QString::number(bytes / (double)TB, 'f', 2) + " TB";
            if (bytes >= GB) return QString::number(bytes / (double)GB, 'f', 2) + " GB";
            if (bytes >= MB) return QString::number(bytes / (double)MB, 'f', 2) + " MB";
            if (bytes >= KB) return QString::number(bytes / (double)KB, 'f', 2) + " KB";
            return QString::number(bytes) + " bytes";
        };
        
        QString spaceInfo = tr("Available: %1 / Total: %2")
            .arg(formatBytes(available))
            .arg(formatBytes(total));
        
        QMessageBox::information(this, tr("Success"), 
                                tr("Cloud destination added successfully!\n\n%1").arg(spaceInfo));
    } else {
        delete destination;
        delete provider;
    }
}

void DestinationTab::onRemoveDestination()
{
    QString destinationId = getSelectedDestinationId();
    if (destinationId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a destination to remove"));
        return;
    }
    
    BackupDestination *dest = m_destinationManager->getDestination(destinationId);
    if (!dest) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirm Removal"),
                                                              tr("Are you sure you want to remove destination: %1?").arg(dest->getPath()),
                                                              QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Remove from file monitor first
        m_backupFileMonitor->removeDestinationPath(destinationId);
        
        if (m_destinationManager->removeDestination(destinationId)) {
            QMessageBox::information(this, tr("Success"), tr("Destination removed successfully"));
        }
    }
}

void DestinationTab::onCheckDestination()
{
    QString destinationId = getSelectedDestinationId();
    if (!destinationId.isEmpty()) {
        m_destinationManager->checkDestination(destinationId);
    }
}

void DestinationTab::onRefreshDestinations()
{
    m_destinationManager->checkAllDestinations();
}

void DestinationTab::onRetentionDaysChanged(int days)
{
    updateRetentionPolicy();
}

void DestinationTab::onAutoCleanupToggled(bool enabled)
{
    updateRetentionPolicy();
}

void DestinationTab::onDestinationAdded(const QString &destinationId)
{
    refreshDestinationTable();
}

void DestinationTab::onDestinationRemoved(const QString &destinationId)
{
    refreshDestinationTable();
}

void DestinationTab::onDestinationUpdated(const QString &destinationId)
{
    refreshDestinationTable();
}

void DestinationTab::onDestinationStatusChanged(const QString &destinationId, DestinationStatus status)
{
    refreshDestinationTable();
}

void DestinationTab::onCheckCompleted(const QString &destinationId, bool success)
{
    if (success) {
        // Status bar or notification can be updated here
    }
}

void DestinationTab::onError(const QString &message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void DestinationTab::refreshDestinationTable()
{
    ui->tableDestinations->setRowCount(0);
    
    QList<BackupDestination*> destinations = m_destinationManager->getAllDestinations();
    
    for (int i = 0; i < destinations.size(); ++i) {
        BackupDestination *dest = destinations[i];
        
        ui->tableDestinations->insertRow(i);
        
        // Type
        auto *typeItem = new QTableWidgetItem(dest->getTypeString());
        typeItem->setData(Qt::UserRole, dest->getId());  // Store ID in user role
        ui->tableDestinations->setItem(i, 0, typeItem);
        
        // Path
        ui->tableDestinations->setItem(i, 1, new QTableWidgetItem(dest->getPath()));
        
        // Free Space
        ui->tableDestinations->setItem(i, 2, new QTableWidgetItem(dest->getFreeSpaceString()));
        
        // Status
        auto *statusItem = new QTableWidgetItem(dest->getStatusString());
        
        // Color code status
        switch (dest->getStatus()) {
            case DestinationStatus::Available:
                statusItem->setForeground(Qt::darkGreen);
                break;
            case DestinationStatus::Unavailable:
                statusItem->setForeground(Qt::red);
                break;
            case DestinationStatus::Checking:
                statusItem->setForeground(Qt::blue);
                break;
            case DestinationStatus::Error:
                statusItem->setForeground(Qt::darkRed);
                break;
        }
        
        ui->tableDestinations->setItem(i, 3, statusItem);
    }
    
    // Resize columns to content
    ui->tableDestinations->resizeColumnsToContents();
}

void DestinationTab::updateRetentionPolicy()
{
    RetentionPolicy policy = m_destinationManager->getRetentionPolicy();
    policy.setRetentionDays(ui->spinRetentionDays->value());
    policy.setAutoCleanup(ui->chkAutoCleanup->isChecked());
    m_destinationManager->setRetentionPolicy(policy);
}

QString DestinationTab::getSelectedDestinationId() const
{
    int currentRow = ui->tableDestinations->currentRow();
    if (currentRow < 0) {
        return QString();
    }
    
    QTableWidgetItem *item = ui->tableDestinations->item(currentRow, 0);
    if (item) {
        return item->data(Qt::UserRole).toString();
    }
    
    return QString();
}

QPushButton* DestinationTab::getBtnBrowseDestination() { return ui->btnBrowseDestination; }
QPushButton* DestinationTab::getBtnAddCloudDest() { return ui->btnAddCloudDest; }
QPushButton* DestinationTab::getBtnRemoveDestination() { return ui->btnRemoveDestination; }

// File monitor slot implementations
void DestinationTab::onFileAdded(const QString &destinationId, const QString &filePath, const BackupFileInfo &info)
{
    qDebug() << "File added in destination" << destinationId << ":" << info.fileName;
    // You can update UI or show notification here
}

void DestinationTab::onFileModified(const QString &destinationId, const QString &filePath, 
                                    const BackupFileInfo &oldInfo, const BackupFileInfo &newInfo)
{
    qDebug() << "File modified in destination" << destinationId << ":" << newInfo.fileName
             << "Old size:" << oldInfo.size << "New size:" << newInfo.size;
    // You can update UI or show notification here
}

void DestinationTab::onFileDeleted(const QString &destinationId, const QString &filePath, const BackupFileInfo &info)
{
    qDebug() << "File deleted from destination" << destinationId << ":" << info.fileName;
    // You can update UI or show notification here
}

void DestinationTab::onScanCompleted(const QString &destinationId, int filesFound, int changesDetected)
{
    qDebug() << "Scan completed for destination" << destinationId 
             << "- Files:" << filesFound << "Changes:" << changesDetected;
    updateMonitoringStatus();
}

void DestinationTab::onChangeDetected(const QString &destinationId, const FileChangeRecord &change)
{
    QString changeType;
    switch (change.changeType) {
        case FileChangeRecord::Added: changeType = "Added"; break;
        case FileChangeRecord::Modified: changeType = "Modified"; break;
        case FileChangeRecord::Deleted: changeType = "Deleted"; break;
        case FileChangeRecord::Renamed: changeType = "Renamed"; break;
        case FileChangeRecord::SizeChanged: changeType = "Size Changed"; break;
    }
    
    qDebug() << "Change detected:" << changeType << "-" << change.description;
}

void DestinationTab::onMonitoringStateChanged(bool enabled)
{
    updateMonitoringStatus();
}

void DestinationTab::onViewChangeHistory()
{
    QString destinationId = getSelectedDestinationId();
    if (destinationId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), 
                           tr("Please select a destination to view change history"));
        return;
    }
    
    QList<FileChangeRecord> changes = m_backupFileMonitor->getChangeHistory(destinationId, 50);
    
    if (changes.isEmpty()) {
        QMessageBox::information(this, tr("Change History"), 
                               tr("No changes recorded for this destination."));
        return;
    }
    
    QString historyText = tr("Recent Changes (Last 50):\n\n");
    for (const FileChangeRecord &change : changes) {
        QString changeType;
        switch (change.changeType) {
            case FileChangeRecord::Added: changeType = "[+]"; break;
            case FileChangeRecord::Modified: changeType = "[M]"; break;
            case FileChangeRecord::Deleted: changeType = "[-]"; break;
            case FileChangeRecord::Renamed: changeType = "[R]"; break;
            case FileChangeRecord::SizeChanged: changeType = "[S]"; break;
        }
        
        historyText += QString("%1 %2 - %3\n")
            .arg(change.changeTime.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(changeType)
            .arg(change.description);
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Change History"));
    msgBox.setText(historyText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.exec();
}

void DestinationTab::onToggleMonitoring(bool enabled)
{
    m_backupFileMonitor->setMonitoringEnabled(enabled);
}

void DestinationTab::updateMonitoringStatus()
{
    int totalFiles = m_backupFileMonitor->getTotalFilesMonitored();
    qint64 totalSize = m_backupFileMonitor->getTotalSizeMonitored();
    int totalChanges = 0;
    
    // Count total changes from all destinations
    QList<BackupDestination*> destinations = m_destinationManager->getAllDestinations();
    for (BackupDestination *dest : destinations) {
        QList<FileChangeRecord> changes = m_backupFileMonitor->getChangeHistory(dest->getId(), 10);
        totalChanges += changes.size();
    }
    
    QString statusText = QString("Monitoring: %1 | Backup Files: %2 | Size: %3 | Changes: %4")
        .arg(m_backupFileMonitor->isMonitoringEnabled() ? 
             "<span style='color: green;'><b>Active</b></span>" : 
             "<span style='color: red;'><b>Inactive</b></span>")
        .arg(totalFiles)
        .arg(formatBytes(totalSize))
        .arg(totalChanges);
    
    ui->lblDestMonitoringStatus->setText(statusText);
    
    // Update background color based on status
    if (m_backupFileMonitor->isMonitoringEnabled()) {
        ui->lblDestMonitoringStatus->setStyleSheet(
            "QLabel { padding: 5px; background-color: #d4edda; border: 1px solid #c3e6cb; "
            "border-radius: 3px; color: #155724; }");
    } else {
        ui->lblDestMonitoringStatus->setStyleSheet(
            "QLabel { padding: 5px; background-color: #f8d7da; border: 1px solid #f5c6cb; "
            "border-radius: 3px; color: #721c24; }");
    }
    
    // Sync checkbox state
    ui->chkEnableDestMonitoring->setChecked(m_backupFileMonitor->isMonitoringEnabled());
    
    qDebug() << "Destination monitoring status updated:" << totalFiles << "files," << totalChanges << "changes";
}

QString DestinationTab::formatBytes(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;
    
    if (bytes >= TB) return QString::number(bytes / (double)TB, 'f', 2) + " TB";
    if (bytes >= GB) return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    if (bytes >= MB) return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    if (bytes >= KB) return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    return QString::number(bytes) + " bytes";
}
