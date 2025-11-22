#include "sourcestab.h"
#include "ui_sourcestab.h"
#include "sourcemanager.h"
#include "networkcredentialsdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>
#include <QHeaderView>

SourcesTab::SourcesTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SourcesTab)
    , m_sourceManager(new SourceManager(this))
{
    ui->setupUi(this);
    
    // Setup table
    ui->tableSourceList->horizontalHeader()->setStretchLastSection(true);
    ui->tableSourceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableSourceList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Connect manager signals
    setupConnections();
    
    // Load saved sources
    m_sourceManager->loadFromFile("sources.json");
    
    // Refresh table
    refreshSourceTable();
}

SourcesTab::~SourcesTab()
{
    // Save sources before destruction
    m_sourceManager->saveToFile("sources.json");
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
    
    // Connect change monitoring settings
    connect(ui->chkMonitorChanges, &QCheckBox::toggled,
            m_sourceManager, &SourceManager::enableChangeMonitoring);
    connect(ui->spinCheckInterval, QOverload<int>::of(&QSpinBox::valueChanged),
            m_sourceManager, &SourceManager::setCheckInterval);
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

QPushButton* SourcesTab::getBtnAddNetwork() { return ui->btnAddNetwork; }
QPushButton* SourcesTab::getBtnAddCloud() { return ui->btnAddCloud; }
QPushButton* SourcesTab::getBtnEditSource() { return ui->btnEditSource; }
QPushButton* SourcesTab::getBtnRemoveSource() { return ui->btnRemoveSource; }
QPushButton* SourcesTab::getBtnTestConnection() { return ui->btnTestConnection; }
