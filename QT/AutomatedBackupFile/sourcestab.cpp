#include "sourcestab.h"
#include "ui_sourcestab.h"
#include "sourcemanager.h"
#include "networkcredentialsdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>

SourcesTab::SourcesTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SourcesTab)
    , m_sourceManager(new SourceManager(this))
    , m_sourceFileMonitor(new BackupFileMonitor(this))
{
    ui->setupUi(this);
    
    // Setup table
    ui->tableSourceList->horizontalHeader()->setStretchLastSection(true);
    ui->tableSourceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableSourceList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Connect manager signals
    setupConnections();
    setupSourceFileMonitorConnections();
    
    // Load saved sources and monitor state
    m_sourceManager->loadFromFile("sources.json");
    m_sourceFileMonitor->loadState("source_file_monitor.json");
    
    // Add existing sources to file monitor
    QList<BackupSource*> sources = m_sourceManager->getAllSources();
    for (BackupSource *source : sources) {
        if (source->getType() == SourceType::Local && source->isEnabled()) {
            m_sourceFileMonitor->addDestinationPath(source->getId(), source->getPath());
        }
    }
    
    // Refresh table
    refreshSourceTable();
    updateSourceMonitoringStatus();
}

SourcesTab::~SourcesTab()
{
    // Save sources and file monitor state before destruction
    m_sourceManager->saveToFile("sources.json");
    m_sourceFileMonitor->saveState("source_file_monitor.json");
    delete ui;
}

void SourcesTab::setupConnections()
{
    // Connect manager signals
    connect(m_sourceManager, &SourceManager::sourceAdded,
            this, [this](const QString &) { refreshSourceTable(); });
    connect(m_sourceManager, &SourceManager::sourceRemoved,
            this, [this](const QString &) { refreshSourceTable(); });
    connect(m_sourceManager, &SourceManager::sourceUpdated,
            this, [this](const QString &) { refreshSourceTable(); });
    connect(m_sourceManager, &SourceManager::error,
            this, [this](const QString &error) {
        QMessageBox::critical(this, tr("Error"), error);
    });
    
    // Connect UI buttons to slots
    connect(ui->btnAddLocal, &QPushButton::clicked, this, &SourcesTab::onAddLocalSource);
    connect(ui->btnAddNetwork, &QPushButton::clicked, this, &SourcesTab::onAddNetworkSource);
    connect(ui->btnAddCloud, &QPushButton::clicked, this, &SourcesTab::onAddCloudSource);
    connect(ui->btnEditSource, &QPushButton::clicked, this, &SourcesTab::onEditSource);
    connect(ui->btnRemoveSource, &QPushButton::clicked, this, &SourcesTab::onRemoveSource);
    connect(ui->btnTestConnection, &QPushButton::clicked, this, &SourcesTab::onTestConnection);
    
    // Connect change monitoring settings
    connect(ui->chkMonitorChanges, &QCheckBox::toggled,
            this, &SourcesTab::onToggleSourceMonitoring);
    connect(ui->spinCheckInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int minutes) { 
                m_sourceFileMonitor->setScanInterval(minutes);
            });
}

void SourcesTab::setupSourceFileMonitorConnections()
{
    // Connect source file monitor signals
    connect(m_sourceFileMonitor, &BackupFileMonitor::fileAdded,
            this, &SourcesTab::onSourceFileAdded);
    connect(m_sourceFileMonitor, &BackupFileMonitor::fileModified,
            this, &SourcesTab::onSourceFileModified);
    connect(m_sourceFileMonitor, &BackupFileMonitor::fileDeleted,
            this, &SourcesTab::onSourceFileDeleted);
    connect(m_sourceFileMonitor, &BackupFileMonitor::scanCompleted,
            this, &SourcesTab::onSourceScanCompleted);
    connect(m_sourceFileMonitor, &BackupFileMonitor::changeDetected,
            this, &SourcesTab::onSourceChangeDetected);
    connect(m_sourceFileMonitor, &BackupFileMonitor::monitoringStateChanged,
            this, &SourcesTab::onSourceMonitoringStateChanged);
}

void SourcesTab::onAddLocalSource()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Select Local Directory"),
                                                     QString(),
                                                     QFileDialog::ShowDirsOnly);
    
    if (path.isEmpty()) {
        return;
    }
    
    // Test if path is accessible
    if (!m_sourceManager->testLocalPath(path)) {
        QMessageBox::warning(this, tr("Invalid Path"),
                           tr("The selected directory is not accessible."));
        return;
    }
    
    auto *source = new BackupSource(path, SourceType::Local);
    
    if (m_sourceManager->addSource(source)) {
        // Add to source file monitor
        m_sourceFileMonitor->addDestinationPath(source->getId(), path);
        QMessageBox::information(this, tr("Success"),
                               tr("Local source added successfully"));
    } else {
        delete source;
    }
}

void SourcesTab::onAddNetworkSource()
{
    bool ok;
    QString path = QInputDialog::getText(this, tr("Add Network Path"),
                                        tr("Enter network path (e.g., \\\\server\\share):"),
                                        QLineEdit::Normal, "", &ok);
    
    if (!ok || path.isEmpty()) {
        return;
    }
    
    auto *source = new BackupSource(path, SourceType::Network);
    
    // Test connectivity first
    bool requiresAuth = false;
    if (!m_sourceManager->testNetworkPath(path)) {
        // Path not accessible, might need credentials
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            tr("Authentication Required"),
            tr("The network path is not accessible. Do you want to provide credentials?"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            NetworkCredentialsDialog dialog(path, this);
            if (dialog.exec() == QDialog::Accepted) {
                QString username = dialog.getUsername();
                QString password = dialog.getPassword();
                QString domain = dialog.getDomain();
                
                if (username.isEmpty()) {
                    QMessageBox::warning(this, tr("Invalid Credentials"),
                                       tr("Username cannot be empty"));
                    delete source;
                    return;
                }
                
                // Test with credentials
                if (m_sourceManager->testNetworkPath(path, username, password, domain)) {
                    source->setUsername(username);
                    source->setPassword(password);
                    source->setDomain(domain);
                    source->setRequiresAuthentication(true);
                    requiresAuth = true;
                } else {
                    QMessageBox::critical(this, tr("Connection Failed"),
                                        tr("Failed to connect to network path with provided credentials."));
                    delete source;
                    return;
                }
            } else {
                delete source;
                return;
            }
        } else {
            delete source;
            return;
        }
    }
    
    if (m_sourceManager->addSource(source)) {
        QString msg = requiresAuth ? 
            tr("Network source added successfully with authentication") :
            tr("Network source added successfully");
        QMessageBox::information(this, tr("Success"), msg);
    } else {
        delete source;
    }
}

void SourcesTab::onAddCloudSource()
{
    QMessageBox::information(this, tr("Cloud Sources"),
                           tr("Cloud source integration uses the backup destinations.\n\n"
                              "Please add cloud destinations in the Destinations tab, "
                              "and they will be available as backup targets."));
}

void SourcesTab::onEditSource()
{
    QString sourceId = getSelectedSourceId();
    if (sourceId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                           tr("Please select a source to edit"));
        return;
    }
    
    BackupSource *source = m_sourceManager->getSource(sourceId);
    if (!source) {
        return;
    }
    
    if (source->getType() == SourceType::Network && source->requiresAuthentication()) {
        // Allow editing credentials
        NetworkCredentialsDialog dialog(source->getPath(), this);
        if (dialog.exec() == QDialog::Accepted) {
            source->setUsername(dialog.getUsername());
            source->setPassword(dialog.getPassword());
            source->setDomain(dialog.getDomain());
            
            // Retest connection
            m_sourceManager->checkSource(source);
        }
    } else {
        QMessageBox::information(this, tr("Edit Source"),
                               tr("Only network sources with authentication can be edited.\n"
                                  "Path: %1").arg(source->getPath()));
    }
}

void SourcesTab::onRemoveSource()
{
    QString sourceId = getSelectedSourceId();
    if (sourceId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                           tr("Please select a source to remove"));
        return;
    }
    
    BackupSource *source = m_sourceManager->getSource(sourceId);
    if (!source) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Confirm Removal"),
        tr("Are you sure you want to remove source:\n%1?").arg(source->getPath()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Remove from file monitor first
        m_sourceFileMonitor->removeDestinationPath(sourceId);
        
        if (m_sourceManager->removeSource(sourceId)) {
            QMessageBox::information(this, tr("Success"),
                                   tr("Source removed successfully"));
        }
    }
}

void SourcesTab::onTestConnection()
{
    QString sourceId = getSelectedSourceId();
    if (sourceId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                           tr("Please select a source to test"));
        return;
    }
    
    BackupSource *source = m_sourceManager->getSource(sourceId);
    if (!source) {
        return;
    }
    
    // Show progress message
    QMessageBox *progressMsg = new QMessageBox(this);
    progressMsg->setWindowTitle(tr("Testing Connection"));
    progressMsg->setText(tr("Testing connection to:\n%1").arg(source->getPath()));
    progressMsg->setStandardButtons(QMessageBox::NoButton);
    progressMsg->show();
    QApplication::processEvents();
    
    // Connect to completion signal
    connect(m_sourceManager, &SourceManager::sourceCheckCompleted,
            this, [this, progressMsg, sourceId](const QString &id, bool success) {
        if (id == sourceId) {
            progressMsg->close();
            delete progressMsg;
            
            BackupSource *src = m_sourceManager->getSource(id);
            if (src) {
                if (success) {
                    QString info = tr("Connection successful!\n\n"
                                    "Path: %1\n"
                                    "Type: %2\n"
                                    "Status: %3")
                        .arg(src->getPath())
                        .arg(src->getTypeString())
                        .arg(src->getStatusString());
                    
                    if (src->getType() == SourceType::Local) {
                        info += tr("\nFiles: %1\nSize: %2")
                            .arg(src->getFileCount())
                            .arg(formatBytes(src->getTotalSize()));
                    }
                    
                    QMessageBox::information(this, tr("Test Successful"), info);
                } else {
                    QMessageBox::critical(this, tr("Test Failed"),
                                        tr("Connection failed:\n%1").arg(src->getLastError()));
                }
            }
        }
    }, Qt::SingleShotConnection);
    
    // Start the test
    m_sourceManager->checkSource(source);
}

void SourcesTab::refreshSourceTable()
{
    ui->tableSourceList->setRowCount(0);
    
    QList<BackupSource*> sources = m_sourceManager->getAllSources();
    
    for (BackupSource *source : sources) {
        int row = ui->tableSourceList->rowCount();
        ui->tableSourceList->insertRow(row);
        
        // Type
        QTableWidgetItem *typeItem = new QTableWidgetItem(source->getTypeString());
        typeItem->setData(Qt::UserRole, source->getId());
        ui->tableSourceList->setItem(row, 0, typeItem);
        
        // Path
        ui->tableSourceList->setItem(row, 1, new QTableWidgetItem(source->getPath()));
        
        // Username
        QString username = source->getUsername();
        if (username.isEmpty()) {
            username = "-";
        }
        ui->tableSourceList->setItem(row, 2, new QTableWidgetItem(username));
        
        // Status with color coding
        QTableWidgetItem *statusItem = new QTableWidgetItem(source->getStatusString());
        switch (source->getStatus()) {
            case SourceStatus::Available:
                statusItem->setForeground(Qt::darkGreen);
                break;
            case SourceStatus::Unavailable:
            case SourceStatus::Error:
                statusItem->setForeground(Qt::red);
                break;
            case SourceStatus::Checking:
                statusItem->setForeground(Qt::blue);
                break;
            case SourceStatus::CredentialsRequired:
                statusItem->setForeground(QColor(255, 140, 0)); // Orange
                break;
        }
        ui->tableSourceList->setItem(row, 3, statusItem);
        
        // Last Checked
        QString lastChecked = source->getLastChecked().isValid() ?
            source->getLastChecked().toString("yyyy-MM-dd hh:mm:ss") : "Never";
        ui->tableSourceList->setItem(row, 4, new QTableWidgetItem(lastChecked));
    }
    
    // Resize columns to content
    ui->tableSourceList->resizeColumnsToContents();
}

QString SourcesTab::getSelectedSourceId() const
{
    int row = ui->tableSourceList->currentRow();
    if (row < 0) {
        return QString();
    }
    
    QTableWidgetItem *item = ui->tableSourceList->item(row, 0);
    if (item) {
        return item->data(Qt::UserRole).toString();
    }
    
    return QString();
}

QString SourcesTab::formatBytes(qint64 bytes) const
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

QPushButton* SourcesTab::getBtnAddLocal() { return ui->btnAddLocal; }
QPushButton* SourcesTab::getBtnAddNetwork() { return ui->btnAddNetwork; }
QPushButton* SourcesTab::getBtnAddCloud() { return ui->btnAddCloud; }
QPushButton* SourcesTab::getBtnEditSource() { return ui->btnEditSource; }
QPushButton* SourcesTab::getBtnRemoveSource() { return ui->btnRemoveSource; }
QPushButton* SourcesTab::getBtnTestConnection() { return ui->btnTestConnection; }

// Source file monitor slot implementations
void SourcesTab::onSourceFileAdded(const QString &sourceId, const QString &filePath, const BackupFileInfo &info)
{
    qDebug() << "File added in source" << sourceId << ":" << info.fileName << "(" << formatBytes(info.size) << ")";
    updateSourceMonitoringStatus();
}

void SourcesTab::onSourceFileModified(const QString &sourceId, const QString &filePath, 
                                      const BackupFileInfo &oldInfo, const BackupFileInfo &newInfo)
{
    qDebug() << "File modified in source" << sourceId << ":" << newInfo.fileName
             << "- Size:" << formatBytes(oldInfo.size) << "->" << formatBytes(newInfo.size);
    updateSourceMonitoringStatus();
}

void SourcesTab::onSourceFileDeleted(const QString &sourceId, const QString &filePath, const BackupFileInfo &info)
{
    qDebug() << "File deleted from source" << sourceId << ":" << info.fileName;
    updateSourceMonitoringStatus();
}

void SourcesTab::onSourceScanCompleted(const QString &sourceId, int filesFound, int changesDetected)
{
    qDebug() << "Source scan completed:" << sourceId 
             << "- Files:" << filesFound << "Changes:" << changesDetected;
    
    // Update the source statistics in the manager as well
    BackupSource *source = m_sourceManager->getSource(sourceId);
    if (source) {
        source->setFileCount(filesFound);
        qint64 totalSize = m_sourceFileMonitor->getSizeInDestination(sourceId);
        source->setTotalSize(totalSize);
    }
    
    updateSourceMonitoringStatus();
    refreshSourceTable();
}

void SourcesTab::onSourceChangeDetected(const QString &sourceId, const FileChangeRecord &change)
{
    QString changeType;
    switch (change.changeType) {
        case FileChangeRecord::Added: changeType = "Added"; break;
        case FileChangeRecord::Modified: changeType = "Modified"; break;
        case FileChangeRecord::Deleted: changeType = "Deleted"; break;
        case FileChangeRecord::Renamed: changeType = "Renamed"; break;
        case FileChangeRecord::SizeChanged: changeType = "Size Changed"; break;
    }
    
    qDebug() << "Source change detected in" << sourceId << ":" << changeType << "-" << change.description;
}

void SourcesTab::onSourceMonitoringStateChanged(bool enabled)
{
    ui->chkMonitorChanges->setChecked(enabled);
    updateSourceMonitoringStatus();
}

void SourcesTab::onViewSourceChangeHistory()
{
    QString sourceId = getSelectedSourceId();
    if (sourceId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), 
                           tr("Please select a source to view change history"));
        return;
    }
    
    QList<FileChangeRecord> changes = m_sourceFileMonitor->getChangeHistory(sourceId, 50);
    
    if (changes.isEmpty()) {
        QMessageBox::information(this, tr("Change History"), 
                               tr("No changes recorded for this source."));
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
    msgBox.setWindowTitle(tr("Source Change History"));
    msgBox.setText(historyText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.exec();
}

void SourcesTab::onToggleSourceMonitoring(bool enabled)
{
    m_sourceFileMonitor->setMonitoringEnabled(enabled);
    
    // Also enable/disable the SourceManager's basic monitoring
    m_sourceManager->enableChangeMonitoring(enabled);
    
    if (enabled) {
        QMessageBox::information(this, tr("Monitoring Enabled"),
                               tr("Source file monitoring is now active.\n\n"
                                  "The system will track all file changes in your source directories."));
    }
}

void SourcesTab::updateSourceMonitoringStatus()
{
    int totalFiles = m_sourceFileMonitor->getTotalFilesMonitored();
    qint64 totalSize = m_sourceFileMonitor->getTotalSizeMonitored();
    
    QString statusText = tr("Source Monitoring: %1 | Total Files: %2 | Total Size: %3")
        .arg(m_sourceFileMonitor->isMonitoringEnabled() ? "Active" : "Inactive")
        .arg(totalFiles)
        .arg(formatBytes(totalSize));
    
    // Update status label if you have one in the UI
    // ui->lblSourceMonitoringStatus->setText(statusText);
    
    qDebug() << statusText;
}
