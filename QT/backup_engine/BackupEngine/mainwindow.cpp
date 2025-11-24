#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Create backup engine
    m_backupEngine = new BackupEngine(this);
    
    // Connect signals for progress updates
    connect(m_backupEngine, &BackupEngine::progressUpdated, this, [this](int progress) {
        ui->progressBar->setValue(progress);
        ui->statusLabel->setText(QString("Progress: %1%").arg(progress));
    });
    
    connect(m_backupEngine, &BackupEngine::fileProcessed, this, [this](const QString& filename) {
        ui->statusLabel->setText(QString("Processing: %1").arg(filename));
    });
    
    connect(m_backupEngine, &BackupEngine::backupCompleted, this, [this]() {
        ui->statusLabel->setText("Backup completed successfully!");
        QMessageBox::information(this, "Backup Complete", "Backup completed successfully!");
    });
    
    connect(m_backupEngine, &BackupEngine::backupFailed, this, [this](const QString& error) {
        ui->statusLabel->setText(QString("Backup failed: %1").arg(error));
        QMessageBox::critical(this, "Backup Failed", error);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_StartBackup_clicked()
{
    QString source_address = ui->sourceLineEdit->text();
    QString destination_address = ui->destinationLineEdit->text();
    
    if (source_address.isEmpty() || destination_address.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter both source and destination paths.");
        return;
    }
    
    ui->statusLabel->setText("Starting backup...");
    ui->progressBar->setValue(0);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    
    m_backupEngine->startBackup(source_address, destination_address);
}

void MainWindow::on_StopBackup_clicked()
{
    m_backupEngine->stopBackup();
    ui->statusLabel->setText("Backup stopped by user");
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

