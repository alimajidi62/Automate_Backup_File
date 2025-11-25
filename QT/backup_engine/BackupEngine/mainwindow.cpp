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
    QString sources = ui->sourceLineEdit->text();
    QString destinations = ui->destinationLineEdit->text();
    
    if (sources.isEmpty() || destinations.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter both source and destination paths.");
        return;
    }
    
    // Parse sources and destinations (separated by semicolons)
    QStringList sourceList = sources.split(';', Qt::SkipEmptyParts);
    QStringList destList = destinations.split(';', Qt::SkipEmptyParts);
    
    if (sourceList.size() != destList.size()) {
        QMessageBox::warning(this, "Path Mismatch", "Number of source and destination paths must match.");
        return;
    }
    sourceList[0]="C:\\BackUp\\source1\\";
    sourceList.append("C:\\BackUp\\source2\\");
    destList[0]="C:\\BackUp\\D1\\";
    destList.append("C:\\BackUp\\D2\\");
    std::vector<std::pair<QString, QString>> pairs;
    for (int i = 0; i < sourceList.size(); ++i) {
        pairs.push_back({sourceList[i].trimmed(), destList[i].trimmed()});
    }
    
    ui->statusLabel->setText("Starting backup...");
    ui->progressBar->setValue(0);
   // ui->startButton->setEnabled(false);
    //ui->stopButton->setEnabled(true);
    
    m_backupEngine->startBackup(pairs);
}

void MainWindow::on_StopBackup_clicked()
{
    m_backupEngine->stopBackup();
    ui->statusLabel->setText("Backup stopped by user");
    //ui->startButton->setEnabled(true);
    //ui->stopButton->setEnabled(false);
}

void MainWindow::on_DecryptBackup_clicked()
{
    QString backupPath = ui->destinationLineEdit->text();
    
    if (backupPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter backup destination path to decrypt.");
        return;
    }
    
    // Parse multiple backup paths (separated by semicolons)
    QStringList backupList = backupPath.split(';', Qt::SkipEmptyParts);
    
    QString keyFilePath = QCoreApplication::applicationDirPath() + "/key.txt";
    FileDecryptor decryptor;
    
    if (!decryptor.loadPasswordFromFile(keyFilePath)) {
        QMessageBox::critical(this, "Decryption Failed", "Failed to load password from key.txt");
        return;
    }
    
    ui->statusLabel->setText("Decrypting backup files...");
    
    bool allSuccess = true;
    for (const QString& backup : backupList) {
        QString encryptedDir = backup.trimmed() + "/encrypted";
        
        if (!QDir(encryptedDir).exists()) {
            QMessageBox::warning(this, "Directory Not Found", "Encrypted directory not found: " + encryptedDir);
            allSuccess = false;
            continue;
        }
        
        if (!decryptor.decryptDirectory(encryptedDir)) {
            QMessageBox::critical(this, "Decryption Failed", "Failed to decrypt: " + encryptedDir);
            allSuccess = false;
        }
    }
    
    if (allSuccess) {
        ui->statusLabel->setText("Decryption completed successfully!");
        QMessageBox::information(this, "Decryption Complete", "All files decrypted successfully to 'decrypted' folder in each backup directory.");
    } else {
        ui->statusLabel->setText("Decryption completed with errors");
    }
}

