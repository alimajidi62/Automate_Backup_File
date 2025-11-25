#ifndef BACKUPENGINE_H
#define BACKUPENGINE_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <atomic>
#include <vector>
#include <utility>
#include "fileencryptor.h"

enum class BackupStatus {
    Idle,
    Running,
    Paused,
    Completed,
    Failed
};

class BackupWorker : public QObject
{
    Q_OBJECT

public:
    explicit BackupWorker(const std::vector<std::pair<QString, QString>>& sourceDestPairs, QObject *parent = nullptr);
    
    void stop();
    BackupStatus getStatus() const { return m_status; }
    int getProgress() const { return m_progress; }
    qint64 getTotalFiles() const { return m_totalFiles; }
    qint64 getProcessedFiles() const { return m_processedFiles; }
    QString getCurrentFile() const { return m_currentFile; }

public slots:
    void startBackup();

signals:
    void progressUpdated(int progress);
    void statusChanged(BackupStatus status);
    void fileProcessed(const QString& filename);
    void backupCompleted();
    void backupFailed(const QString& error);

private:
    std::vector<std::pair<QString, QString>> m_sourceDestPairs;
    std::atomic<BackupStatus> m_status;
    std::atomic<int> m_progress;
    std::atomic<qint64> m_totalFiles;
    std::atomic<qint64> m_processedFiles;
    QString m_currentFile;
    std::atomic<bool> m_shouldStop;

    qint64 countFiles(const QString& path);
    bool copyDirectory(const QString& source, const QString& destination);
    bool copyFile(const QString& source, const QString& destination);
    bool encryptDirectory(const QString& unencryptedDir, const QString& encryptedDir, const QString& keyFilePath);
    bool deleteDirectory(const QString& dirPath);
};

class BackupEngine : public QObject
{
    Q_OBJECT

public:
    explicit BackupEngine(QObject *parent = nullptr);
    ~BackupEngine();

    void startBackup(const std::vector<std::pair<QString, QString>>& sourceDestPairs);
    void stopBackup();
    
    BackupStatus getStatus() const;
    int getProgress() const;
    qint64 getTotalFiles() const;
    qint64 getProcessedFiles() const;
    QString getCurrentFile() const;

signals:
    void progressUpdated(int progress);
    void statusChanged(BackupStatus status);
    void fileProcessed(const QString& filename);
    void backupCompleted();
    void backupFailed(const QString& error);

private:
    QThread* m_thread;
    BackupWorker* m_worker;
};

#endif // BACKUPENGINE_H
