#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "sourcestab.h"
#include "scheduletab.h"
#include "taskstab.h"
#include "destinationtab.h"
#include "settingstab.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeUI();
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
    // Source Management Connections
    connect(sourcesTab->getBtnAddNetwork(), &QPushButton::clicked, this, &MainWindow::onAddNetworkPath);
    connect(sourcesTab->getBtnAddCloud(), &QPushButton::clicked, this, &MainWindow::onAddCloudPath);
    connect(sourcesTab->getBtnRemoveSource(), &QPushButton::clicked, this, &MainWindow::onRemoveSource);
    connect(sourcesTab->getBtnEditSource(), &QPushButton::clicked, this, &MainWindow::onEditSource);
    connect(sourcesTab->getBtnTestConnection(), &QPushButton::clicked, this, &MainWindow::onTestConnection);
    
    // Destination Management Connections
    connect(destinationTab->getBtnBrowseDestination(), &QPushButton::clicked, this, &MainWindow::onBrowseLocalDestination);
    connect(destinationTab->getBtnAddCloudDest(), &QPushButton::clicked, this, &MainWindow::onAddCloudDestination);
    connect(destinationTab->getBtnRemoveDestination(), &QPushButton::clicked, this, &MainWindow::onRemoveDestination);
    
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

// Destination Management Slots
void MainWindow::onBrowseLocalDestination()
{
    statusBar()->showMessage("Browse Local Destination - Not yet implemented");
}

void MainWindow::onAddCloudDestination()
{
    statusBar()->showMessage("Add Cloud Destination - Not yet implemented");
}

void MainWindow::onRemoveDestination()
{
    statusBar()->showMessage("Remove Destination - Not yet implemented");
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
    statusBar()->showMessage("Start Backup - Not yet implemented");
}

void MainWindow::onStopBackup()
{
    statusBar()->showMessage("Stop Backup - Not yet implemented");
}

void MainWindow::onViewBackupHistory()
{
    statusBar()->showMessage("View Backup History - Not yet implemented");
}

// Settings Slots
void MainWindow::onSaveSettings()
{
    statusBar()->showMessage("Settings Saved");
}

void MainWindow::onTestEncryption()
{
    statusBar()->showMessage("Test Encryption - Not yet implemented");
}

