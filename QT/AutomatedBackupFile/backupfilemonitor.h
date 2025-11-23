#ifndef BACKUPFILEMONITOR_H
#define BACKUPFILEMONITOR_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMap>
#include <QDateTime>
#include <QFileInfo>
#include <QString>
#include <QList>

// Structure to hold file information
struct BackupFileInfo {
    QString filePath;
    QString fileName;
    qint64 size;
    QDateTime lastModified;
    QDateTime lastChecked;
    QString checksum;  // MD5 or SHA256
    bool isValid;
    
    BackupFileInfo() 
        : size(0), isValid(false) {}
    
    BackupFileInfo(const QString &path)
        : filePath(path), size(0), isValid(false) 
    {
        QFileInfo info(path);
        if (info.exists()) {
            fileName = info.fileName();
            size = info.size();
            lastModified = info.lastModified();
            lastChecked = QDateTime::currentDateTime();
            isValid = true;
        }
    }
    
    bool operator==(const BackupFileInfo &other) const {
        return filePath == other.filePath;
    }
};

// Structure to track changes
struct FileChangeRecord {
    enum ChangeType {
        Added,
        Modified,
        Deleted,
        Renamed,
        SizeChanged
    };
    
    QString filePath;
    ChangeType changeType;
    QDateTime changeTime;
    BackupFileInfo oldInfo;
    BackupFileInfo newInfo;
    QString description;
    
    FileChangeRecord()
        : changeType(Modified), changeTime(QDateTime::currentDateTime()) {}
};

class BackupFileMonitor : public QObject
{
    Q_OBJECT

public:
    explicit BackupFileMonitor(QObject *parent = nullptr);
    ~BackupFileMonitor();
    
    // Monitoring control
    void addDestinationPath(const QString &destinationId, const QString &path);
    void removeDestinationPath(const QString &destinationId);
    void clearAllPaths();
    
    // Enable/disable monitoring
    void setMonitoringEnabled(bool enabled);
    bool isMonitoringEnabled() const { return m_monitoringEnabled; }
    
    // Scan interval (in minutes)
    void setScanInterval(int minutes);
    int getScanInterval() const { return m_scanIntervalMinutes; }
    
    // Manual operations
    void scanDestination(const QString &destinationId);
    void scanAllDestinations();
    void forceRescan();
    
    // File information retrieval
    QList<BackupFileInfo> getFilesInDestination(const QString &destinationId) const;
    QList<FileChangeRecord> getChangeHistory(const QString &destinationId, int maxRecords = 100) const;
    QList<FileChangeRecord> getRecentChanges(int minutes = 60) const;
    
    // Statistics
    int getTotalFilesMonitored() const;
    int getFileCountInDestination(const QString &destinationId) const;
    qint64 getTotalSizeMonitored() const;
    qint64 getSizeInDestination(const QString &destinationId) const;
    QDateTime getLastScanTime(const QString &destinationId) const;
    
    // File validation
    bool verifyFileIntegrity(const QString &filePath);
    QStringList findCorruptedFiles(const QString &destinationId);
    
    // Persistence
    bool saveState(const QString &filePath);
    bool loadState(const QString &filePath);

signals:
    void fileAdded(const QString &destinationId, const QString &filePath, const BackupFileInfo &info);
    void fileModified(const QString &destinationId, const QString &filePath, 
                     const BackupFileInfo &oldInfo, const BackupFileInfo &newInfo);
    void fileDeleted(const QString &destinationId, const QString &filePath, const BackupFileInfo &info);
    void fileRenamed(const QString &destinationId, const QString &oldPath, const QString &newPath);
    void sizeChanged(const QString &destinationId, const QString &filePath, qint64 oldSize, qint64 newSize);
    
    void scanStarted(const QString &destinationId);
    void scanCompleted(const QString &destinationId, int filesFound, int changesDetected);
    void scanError(const QString &destinationId, const QString &error);
    
    void changeDetected(const QString &destinationId, const FileChangeRecord &change);
    void corruptedFileFound(const QString &filePath, const QString &reason);
    
    void monitoringStateChanged(bool enabled);
    void error(const QString &errorMessage);

private slots:
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);
    void onScanTimerTimeout();

private:
    struct DestinationMonitorInfo {
        QString destinationId;
        QString path;
        QMap<QString, BackupFileInfo> files;  // filePath -> info
        QList<FileChangeRecord> changeHistory;
        QDateTime lastScan;
        int fileCount;
        qint64 totalSize;
        
        DestinationMonitorInfo() : fileCount(0), totalSize(0) {}
    };
    
    QFileSystemWatcher *m_fileWatcher;
    QTimer *m_scanTimer;
    bool m_monitoringEnabled;
    int m_scanIntervalMinutes;
    
    QMap<QString, DestinationMonitorInfo> m_destinations;  // destinationId -> monitor info
    QMap<QString, QString> m_pathToDestinationMap;  // path -> destinationId
    
    // Internal methods
    void scanDestinationInternal(DestinationMonitorInfo &destInfo);
    void scanDirectory(const QString &dirPath, QList<BackupFileInfo> &fileList);
    void detectChanges(DestinationMonitorInfo &destInfo, const QList<BackupFileInfo> &currentFiles);
    void recordChange(DestinationMonitorInfo &destInfo, const FileChangeRecord &change);
    
    QString calculateChecksum(const QString &filePath);
    bool isBackupFile(const QString &fileName) const;
    QString findDestinationIdByPath(const QString &path) const;
    
    void startWatching(const QString &path);
    void stopWatching(const QString &path);
};

#endif // BACKUPFILEMONITOR_H
