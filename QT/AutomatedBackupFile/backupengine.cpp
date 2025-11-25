#include "backupengine.h"
#include <QDebug>
#include <QCoreApplication>
#include <algorithm>

// BackupWorker Implementation
// Accepts a vector of (source, destination) pairs
BackupWorker::BackupWorker(const std::vector<std::pair<QString, QString>>& sourceDestPairs, QObject *parent)
    : QObject(parent)
    , m_sourceDestPairs(sourceDestPairs)
    , m_status(BackupStatus::Idle)
    , m_progress(0)
    , m_totalFiles(0)
    , m_processedFiles(0)
    , m_shouldStop(false)
{
}

void BackupWorker::stop()
{
    m_shouldStop = true;
}

qint64 BackupWorker::countFiles(const QString& path)
{
    qint64 count = 0;
    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        count++;
    }
    return count;
}

bool BackupWorker::copyFile(const QString& source, const QString& destination)
{
    QFileInfo fileInfo(destination);
    QDir dir = fileInfo.dir();
    
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            return false;
        }
    }

    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }

    return QFile::copy(source, destination);
}

bool BackupWorker::deleteDirectory(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return true;
    }
    
    // Remove all files and subdirectories
    bool success = dir.removeRecursively();
    if (success) {
        qDebug() << "Deleted unencrypted directory:" << dirPath;
    } else {
        qWarning() << "Failed to delete directory:" << dirPath;
    }
    return success;
}

bool BackupWorker::encryptDirectory(const QString& unencryptedDir, const QString& encryptedDir, const QString& keyFilePath)
{
    FileEncryptor encryptor;
    
    // Load password from key.txt
    if (!encryptor.loadPasswordFromFile(keyFilePath)) {
        qWarning() << "Failed to load encryption password";
        return false;
    }
    
    emit fileProcessed("Encrypting files...");
    bool success = encryptor.encryptDirectory(unencryptedDir, encryptedDir);
    
    if (success) {
        qDebug() << "Encryption completed for:" << unencryptedDir;
    } else {
        qWarning() << "Encryption failed for:" << unencryptedDir;
    }
    
    return success;
}

bool BackupWorker::copyDirectory(const QString& source, const QString& destination)
{
    QDir sourceDir(source);
    if (!sourceDir.exists()) {
        return false;
    }

    QDir destDir(destination);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QDirIterator it(source, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (it.hasNext() && !m_shouldStop) {
        QString sourceFile = it.next();
        QString relativePath = sourceDir.relativeFilePath(sourceFile);
        QString destFile = destination + "/" + relativePath;

        m_currentFile = relativePath;
        emit fileProcessed(relativePath);

        if (!copyFile(sourceFile, destFile)) {
            qWarning() << "Failed to copy:" << sourceFile;
        }

        m_processedFiles++;
        int progress = (m_totalFiles > 0) ? (m_processedFiles * 100 / m_totalFiles) : 0;
        m_progress = progress;
        emit progressUpdated(progress);
    }

    return !m_shouldStop;
}

void BackupWorker::startBackup()
{
    m_status = BackupStatus::Running;
    emit statusChanged(m_status);
    m_progress = 0;
    m_processedFiles = 0;
    m_shouldStop = false;

    // Count total files for all pairs
    emit fileProcessed("Counting files...");
    m_totalFiles = 0;
    for (const auto& pair : m_sourceDestPairs) {
        m_totalFiles += countFiles(pair.first);
    }
    if (m_totalFiles == 0) {
        m_status = BackupStatus::Failed;
        emit statusChanged(m_status);
        emit backupFailed("No files found in source directories");
        return;
    }

    // Process each pair: Copy -> Encrypt -> Delete unencrypted
    bool allSuccess = true;
    QString keyFilePath = QCoreApplication::applicationDirPath() + "/key.txt";
    
    for (const auto& pair : m_sourceDestPairs) {
        if (m_shouldStop) break;
        
        QString source = pair.first;
        QString destination = pair.second;
        QString tempUnencrypted = destination + "/temp_unencrypted";
        QString encrypted = destination + "/encrypted";
        
        // Step 1: Copy files to temporary location
        emit fileProcessed("Copying from " + source + "...");
        if (!copyDirectory(source, tempUnencrypted)) {
            qWarning() << "Failed to copy directory:" << source;
            allSuccess = false;
            continue;
        }
        
        if (m_shouldStop) break;
        
        // Step 2: Encrypt the copied files
        emit fileProcessed("Encrypting files...");
        if (!encryptDirectory(tempUnencrypted, encrypted, keyFilePath)) {
            qWarning() << "Failed to encrypt directory:" << tempUnencrypted;
            allSuccess = false;
            continue;
        }
        
        if (m_shouldStop) break;
        
        // Step 3: Delete unencrypted files
        emit fileProcessed("Cleaning up unencrypted files...");
        if (!deleteDirectory(tempUnencrypted)) {
            qWarning() << "Failed to delete unencrypted directory:" << tempUnencrypted;
            // Continue anyway, encryption is done
        }
    }

    if (m_shouldStop) {
        m_status = BackupStatus::Failed;
        emit statusChanged(m_status);
        emit backupFailed("Backup cancelled by user");
    } else if (allSuccess) {
        m_status = BackupStatus::Completed;
        m_progress = 100;
        emit progressUpdated(100);
        emit statusChanged(m_status);
        emit backupCompleted();
    } else {
        m_status = BackupStatus::Failed;
        emit statusChanged(m_status);
        emit backupFailed("Backup failed");
    }
}

// BackupEngine Implementation
BackupEngine::BackupEngine(QObject *parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
{
}

BackupEngine::~BackupEngine()
{
    stopBackup();
}

void BackupEngine::startBackup(const std::vector<std::pair<QString, QString>>& sourceDestPairs)
{
    if (m_thread && m_thread->isRunning()) {
        qWarning() << "Backup already running";
        return;
    }

    m_thread = new QThread(this);
    m_worker = new BackupWorker(sourceDestPairs);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &BackupWorker::startBackup);
    connect(m_worker, &BackupWorker::progressUpdated, this, &BackupEngine::progressUpdated);
    connect(m_worker, &BackupWorker::statusChanged, this, &BackupEngine::statusChanged);
    connect(m_worker, &BackupWorker::fileProcessed, this, &BackupEngine::fileProcessed);
    connect(m_worker, &BackupWorker::backupCompleted, this, &BackupEngine::backupCompleted);
    connect(m_worker, &BackupWorker::backupFailed, this, &BackupEngine::backupFailed);
    
    connect(m_worker, &BackupWorker::backupCompleted, m_thread, &QThread::quit);
    connect(m_worker, &BackupWorker::backupFailed, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();
}

void BackupEngine::stopBackup()
{
    if (m_worker) {
        m_worker->stop();
    }
    
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
}

BackupStatus BackupEngine::getStatus() const
{
    return m_worker ? m_worker->getStatus() : BackupStatus::Idle;
}

int BackupEngine::getProgress() const
{
    return m_worker ? m_worker->getProgress() : 0;
}

qint64 BackupEngine::getTotalFiles() const
{
    return m_worker ? m_worker->getTotalFiles() : 0;
}

qint64 BackupEngine::getProcessedFiles() const
{
    return m_worker ? m_worker->getProcessedFiles() : 0;
}

QString BackupEngine::getCurrentFile() const
{
    return m_worker ? m_worker->getCurrentFile() : QString();
}
