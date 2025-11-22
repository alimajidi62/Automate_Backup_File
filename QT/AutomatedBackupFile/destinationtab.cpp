#include "destinationtab.h"
#include "ui_destinationtab.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>

DestinationTab::DestinationTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DestinationTab)
    , m_destinationManager(new DestinationManager(this))
{
    ui->setupUi(this);
    setupConnections();
    
    // Initialize retention policy UI
    ui->spinRetentionDays->setValue(m_destinationManager->getRetentionPolicy().getRetentionDays());
    ui->chkAutoCleanup->setChecked(m_destinationManager->getRetentionPolicy().isAutoCleanupEnabled());
    
    // Setup table
    ui->tableDestinations->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableDestinations->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableDestinations->horizontalHeader()->setStretchLastSection(true);
    
    // Load saved destinations
    m_destinationManager->loadFromFile("destinations.json");
    refreshDestinationTable();
}

DestinationTab::~DestinationTab()
{
    // Save destinations before destroying
    m_destinationManager->saveToFile("destinations.json");
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
    
    // Manager connections
    connect(m_destinationManager, &DestinationManager::destinationAdded, this, &DestinationTab::onDestinationAdded);
    connect(m_destinationManager, &DestinationManager::destinationRemoved, this, &DestinationTab::onDestinationRemoved);
    connect(m_destinationManager, &DestinationManager::destinationUpdated, this, &DestinationTab::onDestinationUpdated);
    connect(m_destinationManager, &DestinationManager::destinationStatusChanged, this, &DestinationTab::onDestinationStatusChanged);
    connect(m_destinationManager, &DestinationManager::checkCompleted, this, &DestinationTab::onCheckCompleted);
    connect(m_destinationManager, &DestinationManager::error, this, &DestinationTab::onError);
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
        QMessageBox::information(this, tr("Success"), tr("Destination added successfully"));
    } else {
        delete destination;
    }
}

void DestinationTab::onAddCloudDestination()
{
    QStringList cloudProviders = {"Google Drive", "OneDrive", "Dropbox", "AWS S3", "Custom"};
    
    bool ok;
    QString provider = QInputDialog::getItem(this, tr("Cloud Provider"),
                                             tr("Select cloud provider:"),
                                             cloudProviders, 0, false, &ok);
    
    if (!ok || provider.isEmpty()) {
        return;
    }
    
    QString path = QInputDialog::getText(this, tr("Cloud Path"),
                                         tr("Enter cloud path or URL:"),
                                         QLineEdit::Normal, "", &ok);
    
    if (!ok || path.isEmpty()) {
        return;
    }
    
    auto *destination = new BackupDestination(path, DestinationType::Cloud);
    
    // Get credentials if needed
    QString username = QInputDialog::getText(this, tr("Username"),
                                            tr("Enter username (optional):"),
                                            QLineEdit::Normal, "", &ok);
    if (ok && !username.isEmpty()) {
        destination->setUsername(username);
        
        QString password = QInputDialog::getText(this, tr("Password"),
                                                tr("Enter password:"),
                                                QLineEdit::Password, "", &ok);
        if (ok) {
            destination->setPassword(password);
        }
    }
    
    if (m_destinationManager->addDestination(destination)) {
        QMessageBox::information(this, tr("Success"), 
                                tr("Cloud destination added. Note: Cloud integration is not yet fully implemented."));
    } else {
        delete destination;
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
        m_destinationManager->removeDestination(destinationId);
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
