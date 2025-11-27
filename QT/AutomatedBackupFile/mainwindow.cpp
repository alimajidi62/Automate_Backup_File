#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "sourcestab.h"
#include "scheduletab.h"
#include "taskstab.h"
#include "destinationtab.h"
#include "settingstab.h"
#include "fileencryptor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QCoreApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_sourceManager(nullptr)
    , m_destinationManager(nullptr)
{
    ui->setupUi(this);
    
    // Initialize UI first (creates tab widgets with their managers)
    initializeUI();
    
    // Get managers from tabs
    m_sourceManager = sourcesTab->getSourceManager();
    m_destinationManager = destinationTab->getDestinationManager();
    
    // Initialize backup engine
    m_backupEngine = new BackupEngine(this);
    
    // Connect backup engine signals
    connect(m_backupEngine, &BackupEngine::progressUpdated, this, &MainWindow::updateBackupProgress);
    connect(m_backupEngine, &BackupEngine::fileProcessed, this, [this](const QString& filename) {
        statusBar()->showMessage("Processing: " + filename);
        tasksTab->getStatusLabel()->setText("Status: Processing " + filename);
    });
    connect(m_backupEngine, &BackupEngine::backupCompleted, this, [this]() {
        statusBar()->showMessage("Backup completed successfully!");
        tasksTab->getStatusLabel()->setText("Status: Backup completed successfully!");
        tasksTab->getProgressBar()->setValue(100);
        QMessageBox::information(this, "Backup Complete", "Backup completed successfully!");
        tasksTab->getBtnStartBackup()->setEnabled(true);
        tasksTab->getBtnStopBackup()->setEnabled(false);
    });
    connect(m_backupEngine, &BackupEngine::backupFailed, this, [this](const QString& error) {
        statusBar()->showMessage("Backup failed: " + error);
        tasksTab->getStatusLabel()->setText("Status: Backup failed - " + error);
        QMessageBox::critical(this, "Backup Failed", error);
        tasksTab->getBtnStartBackup()->setEnabled(true);
        tasksTab->getBtnStopBackup()->setEnabled(false);
    });
    
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeUI()
{
    // Set window title
    setWindowTitle("Automated Backup File Manager");
    
    // Create tab widgets
    sourcesTab = new SourcesTab(this);
    scheduleTab = new ScheduleTab(this);
    tasksTab = new TasksTab(this);
    destinationTab = new DestinationTab(this);
    settingsTab = new SettingsTab(this);
    
    // Add tabs to the tab widget
    ui->tabWidget->addTab(sourcesTab, "Backup Sources");
    ui->tabWidget->addTab(scheduleTab, "Schedule");
    ui->tabWidget->addTab(tasksTab, "Backup Tasks");
    ui->tabWidget->addTab(destinationTab, "Destination");
    ui->tabWidget->addTab(settingsTab, "Settings");
    
    // Initialize status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::setupConnections()
{
    // Source Management - handled internally by SourcesTab
    // No connections needed here as SourcesTab manages its own backend
    
    // Destination Management - handled internally by DestinationTab
    // No connections needed here as DestinationTab manages its own backend
    
    // Schedule Management Connections
    connect(scheduleTab->getBtnAddSchedule(), &QPushButton::clicked, this, &MainWindow::onAddSchedule);
    connect(scheduleTab->getBtnEditSchedule(), &QPushButton::clicked, this, &MainWindow::onEditSchedule);
    connect(scheduleTab->getBtnRemoveSchedule(), &QPushButton::clicked, this, &MainWindow::onRemoveSchedule);
    connect(scheduleTab->getChkEnableScheduler(), &QCheckBox::toggled, this, &MainWindow::onEnableSchedule);
    
    // Backup Operations Connections
    connect(tasksTab->getBtnStartBackup(), &QPushButton::clicked, this, &MainWindow::onStartBackup);
    connect(tasksTab->getBtnStopBackup(), &QPushButton::clicked, this, &MainWindow::onStopBackup);
    connect(tasksTab->getBtnViewHistory(), &QPushButton::clicked, this, &MainWindow::onViewBackupHistory);
    
    // Settings Connections
    connect(settingsTab->getBtnSaveSettings(), &QPushButton::clicked, this, &MainWindow::onSaveSettings);
    connect(settingsTab->getBtnTestEncryption(), &QPushButton::clicked, this, &MainWindow::onTestEncryption);
    
    // Schedule triggered connections
    connect(scheduleTab->getScheduleManager(), &ScheduleManager::scheduleTriggered, 
            this, &MainWindow::onScheduleTriggered);
}

// Source Management Slots
void MainWindow::onAddNetworkPath()
{
    statusBar()->showMessage("Add Network Path - Not yet implemented");
}

void MainWindow::onAddCloudPath()
{
    statusBar()->showMessage("Add Cloud Path - Not yet implemented");
}

void MainWindow::onRemoveSource()
{
    statusBar()->showMessage("Remove Source - Not yet implemented");
}

void MainWindow::onEditSource()
{
    statusBar()->showMessage("Edit Source - Not yet implemented");
}

void MainWindow::onTestConnection()
{
    statusBar()->showMessage("Test Connection - Not yet implemented");
}

// Destination Management Slots - Now handled by DestinationTab
void MainWindow::onBrowseLocalDestination()
{
    statusBar()->showMessage("Destination management is handled in the Destination tab");
}

void MainWindow::onAddCloudDestination()
{
    statusBar()->showMessage("Destination management is handled in the Destination tab");
}

void MainWindow::onRemoveDestination()
{
    statusBar()->showMessage("Destination management is handled in the Destination tab");
}

// Schedule Management Slots
void MainWindow::onAddSchedule()
{
    statusBar()->showMessage("Add Schedule - Not yet implemented");
}

void MainWindow::onEditSchedule()
{
    statusBar()->showMessage("Edit Schedule - Not yet implemented");
}

void MainWindow::onRemoveSchedule()
{
    statusBar()->showMessage("Remove Schedule - Not yet implemented");
}

void MainWindow::onEnableSchedule(bool enabled)
{
    if (enabled) {
        statusBar()->showMessage("Scheduler Enabled");
    } else {
        statusBar()->showMessage("Scheduler Disabled");
    }
}

// Backup Operations Slots
void MainWindow::onStartBackup()
{
    // Get enabled sources and destinations
    QList<BackupSource*> sources = m_sourceManager->getEnabledSources();
    QList<BackupDestination*> destinations = m_destinationManager->getAllDestinations();
    
    if (sources.isEmpty()) {
        QMessageBox::warning(this, "No Sources", "Please add at least one backup source in the 'Backup Sources' tab.");
        return;
    }
    
    if (destinations.isEmpty()) {
        QMessageBox::warning(this, "No Destinations", "Please add at least one destination in the 'Destination' tab.");
        return;
    }
    
    // Build source-destination pairs
    std::vector<std::pair<QString, QString>> pairs;
    
    for (BackupSource* source : sources) {
        if (!source->isEnabled()) continue;
        
        for (BackupDestination* dest : destinations) {
            if (dest->getStatus() != DestinationStatus::Available) continue;
            
            QString sourcePath = source->getPath();
            QString destPath = dest->getPath() + "/" + source->getId();
            
            pairs.push_back({sourcePath, destPath});
        }
    }
    
    if (pairs.empty()) {
        QMessageBox::warning(this, "No Valid Pairs", "No valid source-destination pairs found. Check source and destination status.");
        return;
    }
    
    // Update UI
    tasksTab->getBtnStartBackup()->setEnabled(false);
    tasksTab->getBtnStopBackup()->setEnabled(true);
    tasksTab->getProgressBar()->setValue(0);
    tasksTab->getStatusLabel()->setText("Status: Starting backup...");
    statusBar()->showMessage("Starting backup...");
    
    // Start backup
    m_backupEngine->startBackup(pairs);
}

void MainWindow::onStopBackup()
{
    m_backupEngine->stopBackup();
    statusBar()->showMessage("Backup stopped by user");
    tasksTab->getBtnStartBackup()->setEnabled(true);
    tasksTab->getBtnStopBackup()->setEnabled(false);
}

void MainWindow::onViewBackupHistory()
{
    statusBar()->showMessage("View Backup History - Not yet implemented");
}

void MainWindow::onScheduleTriggered(const QString &scheduleId, const QString &scheduleName)
{
    qDebug() << "Schedule triggered:" << scheduleName << "(" << scheduleId << ")";
    
    // Mark schedule as run
    scheduleTab->getScheduleManager()->markScheduleRun(scheduleId);
    
    // Automatically start backup
    statusBar()->showMessage("Scheduled backup started: " + scheduleName);
    onStartBackup();
}

void MainWindow::updateBackupProgress(int progress)
{
    statusBar()->showMessage(QString("Backup Progress: %1%").arg(progress));
    tasksTab->getProgressBar()->setValue(progress);
    tasksTab->getStatusLabel()->setText(QString("Status: Backing up... %1%").arg(progress));
}

// Settings Slots
void MainWindow::onSaveSettings()
{
    statusBar()->showMessage("Settings Saved");
}

void MainWindow::onTestEncryption()
{
    QString keyFilePath = QCoreApplication::applicationDirPath() + "/key.txt";
    
    if (!QFile::exists(keyFilePath)) {
        QMessageBox::warning(this, "Key File Missing", 
            "Encryption key file not found at: " + keyFilePath + 
            "\n\nPlease create a key.txt file with your encryption password.");
        return;
    }
    
    FileEncryptor encryptor;
    if (encryptor.loadPasswordFromFile(keyFilePath)) {
        QMessageBox::information(this, "Encryption Test", 
            "Encryption key loaded successfully from key.txt\n\nEncryption is ready to use.");
        statusBar()->showMessage("Encryption test passed");
    } else {
        QMessageBox::critical(this, "Encryption Test Failed", 
            "Failed to load encryption key from key.txt");
        statusBar()->showMessage("Encryption test failed");
    }
}

