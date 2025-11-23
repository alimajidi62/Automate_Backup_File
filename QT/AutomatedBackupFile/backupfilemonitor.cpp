#include "backupfilemonitor.h"
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

BackupFileMonitor::BackupFileMonitor(QObject *parent)
    : QObject(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_scanTimer(new QTimer(this))
    , m_monitoringEnabled(false)
    , m_scanIntervalMinutes(30)  // Default: 30 minutes
{
    // Connect file system watcher signals
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &BackupFileMonitor::onDirectoryChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &BackupFileMonitor::onFileChanged);
    
    // Connect scan timer
    connect(m_scanTimer, &QTimer::timeout,
            this, &BackupFileMonitor::onScanTimerTimeout);
    
    // Set timer interval (convert minutes to milliseconds)
    m_scanTimer->setInterval(m_scanIntervalMinutes * 60 * 1000);
}

BackupFileMonitor::~BackupFileMonitor()
{
    clearAllPaths();
}

void BackupFileMonitor::addDestinationPath(const QString &destinationId, const QString &path)
{
    if (destinationId.isEmpty() || path.isEmpty()) {
        emit error("Invalid destination ID or path");
        return;
    }
    
    // Check if path exists
    QDir dir(path);
    if (!dir.exists()) {
        emit error(QString("Destination path does not exist: %1").arg(path));
        return;
    }
    
    // Create or update destination info
    DestinationMonitorInfo destInfo;
    destInfo.destinationId = destinationId;
    destInfo.path = path;
    
    m_destinations[destinationId] = destInfo;
    m_pathToDestinationMap[path] = destinationId;
    
    // Start watching the directory
    startWatching(path);
    
    // Perform initial scan
    scanDestination(destinationId);
}

void BackupFileMonitor::removeDestinationPath(const QString &destinationId)
{
    if (!m_destinations.contains(destinationId)) {
        return;
    }
    
    DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    
    // Stop watching
    stopWatching(destInfo.path);
    
    // Remove from maps
    m_pathToDestinationMap.remove(destInfo.path);
    m_destinations.remove(destinationId);
}

void BackupFileMonitor::clearAllPaths()
{
    // Stop all watchers
    if (!m_fileWatcher->directories().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->directories());
    }
    if (!m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }
    
    m_destinations.clear();
    m_pathToDestinationMap.clear();
}

void BackupFileMonitor::setMonitoringEnabled(bool enabled)
{
    if (m_monitoringEnabled == enabled) {
        return;
    }
    
    m_monitoringEnabled = enabled;
    
    if (enabled) {
        m_scanTimer->start();
        // Perform immediate scan
        scanAllDestinations();
    } else {
        m_scanTimer->stop();
    }
    
    emit monitoringStateChanged(enabled);
}

void BackupFileMonitor::setScanInterval(int minutes)
{
    if (minutes < 1) {
        minutes = 1;
    }
    
    m_scanIntervalMinutes = minutes;
    m_scanTimer->setInterval(minutes * 60 * 1000);
    
    // Restart timer if running
    if (m_scanTimer->isActive()) {
        m_scanTimer->start();
    }
}

void BackupFileMonitor::scanDestination(const QString &destinationId)
{
    if (!m_destinations.contains(destinationId)) {
        emit error(QString("Destination not found: %1").arg(destinationId));
        return;
    }
    
    emit scanStarted(destinationId);
    
    DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    scanDestinationInternal(destInfo);
}

void BackupFileMonitor::scanAllDestinations()
{
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        scanDestination(it.key());
    }
}

void BackupFileMonitor::forceRescan()
{
    // Clear all file info and rescan
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        it->files.clear();
        it->fileCount = 0;
        it->totalSize = 0;
    }
    
    scanAllDestinations();
}

void BackupFileMonitor::scanDestinationInternal(DestinationMonitorInfo &destInfo)
{
    QList<BackupFileInfo> currentFiles;
    
    try {
        scanDirectory(destInfo.path, currentFiles);
    } catch (const std::exception &e) {
        emit scanError(destInfo.destinationId, QString("Scan failed: %1").arg(e.what()));
        return;
    }
    
    // Detect changes
    int changeCount = 0;
    if (!destInfo.files.isEmpty()) {
        detectChanges(destInfo, currentFiles);
        changeCount = destInfo.changeHistory.size();
    }
    
    // Update file list
    destInfo.files.clear();
    destInfo.fileCount = 0;
    destInfo.totalSize = 0;
    
    for (const BackupFileInfo &fileInfo : currentFiles) {
        destInfo.files[fileInfo.filePath] = fileInfo;
        destInfo.fileCount++;
        destInfo.totalSize += fileInfo.size;
    }
    
    destInfo.lastScan = QDateTime::currentDateTime();
    
    emit scanCompleted(destInfo.destinationId, currentFiles.size(), changeCount);
}

void BackupFileMonitor::scanDirectory(const QString &dirPath, QList<BackupFileInfo> &fileList)
{
    QDirIterator it(dirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        // Only process backup files (you can customize this filter)
        if (isBackupFile(fileInfo.fileName())) {
            BackupFileInfo info(filePath);
            if (info.isValid) {
                fileList.append(info);
            }
        }
    }
}

void BackupFileMonitor::detectChanges(DestinationMonitorInfo &destInfo, const QList<BackupFileInfo> &currentFiles)
{
    QMap<QString, BackupFileInfo> currentFilesMap;
    
    // Build map of current files
    for (const BackupFileInfo &fileInfo : currentFiles) {
        currentFilesMap[fileInfo.filePath] = fileInfo;
    }
    
    // Detect deleted files
    for (auto it = destInfo.files.begin(); it != destInfo.files.end(); ++it) {
        const QString &filePath = it.key();
        const BackupFileInfo &oldInfo = it.value();
        
        if (!currentFilesMap.contains(filePath)) {
            // File was deleted
            FileChangeRecord change;
            change.filePath = filePath;
            change.changeType = FileChangeRecord::Deleted;
            change.changeTime = QDateTime::currentDateTime();
            change.oldInfo = oldInfo;
            change.description = QString("File deleted: %1").arg(oldInfo.fileName);
            
            recordChange(destInfo, change);
            emit fileDeleted(destInfo.destinationId, filePath, oldInfo);
        }
    }
    
    // Detect new and modified files
    for (auto it = currentFilesMap.begin(); it != currentFilesMap.end(); ++it) {
        const QString &filePath = it.key();
        const BackupFileInfo &newInfo = it.value();
        
        if (!destInfo.files.contains(filePath)) {
            // New file
            FileChangeRecord change;
            change.filePath = filePath;
            change.changeType = FileChangeRecord::Added;
            change.changeTime = QDateTime::currentDateTime();
            change.newInfo = newInfo;
            change.description = QString("New file added: %1 (%2 bytes)")
                .arg(newInfo.fileName)
                .arg(newInfo.size);
            
            recordChange(destInfo, change);
            emit fileAdded(destInfo.destinationId, filePath, newInfo);
        } else {
            // Check if modified
            const BackupFileInfo &oldInfo = destInfo.files[filePath];
            
            if (oldInfo.lastModified != newInfo.lastModified) {
                FileChangeRecord change;
                change.filePath = filePath;
                change.changeType = FileChangeRecord::Modified;
                change.changeTime = QDateTime::currentDateTime();
                change.oldInfo = oldInfo;
                change.newInfo = newInfo;
                change.description = QString("File modified: %1 (size: %2 -> %3)")
                    .arg(newInfo.fileName)
                    .arg(oldInfo.size)
                    .arg(newInfo.size);
                
                recordChange(destInfo, change);
                emit fileModified(destInfo.destinationId, filePath, oldInfo, newInfo);
                
                if (oldInfo.size != newInfo.size) {
                    emit sizeChanged(destInfo.destinationId, filePath, oldInfo.size, newInfo.size);
                }
            }
        }
    }
}

void BackupFileMonitor::recordChange(DestinationMonitorInfo &destInfo, const FileChangeRecord &change)
{
    destInfo.changeHistory.prepend(change);
    
    // Limit history to 1000 records per destination
    if (destInfo.changeHistory.size() > 1000) {
        destInfo.changeHistory.removeLast();
    }
    
    emit changeDetected(destInfo.destinationId, change);
}

QList<BackupFileInfo> BackupFileMonitor::getFilesInDestination(const QString &destinationId) const
{
    if (!m_destinations.contains(destinationId)) {
        return QList<BackupFileInfo>();
    }
    
    const DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    return destInfo.files.values();
}

QList<FileChangeRecord> BackupFileMonitor::getChangeHistory(const QString &destinationId, int maxRecords) const
{
    if (!m_destinations.contains(destinationId)) {
        return QList<FileChangeRecord>();
    }
    
    const DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    
    if (maxRecords <= 0 || maxRecords >= destInfo.changeHistory.size()) {
        return destInfo.changeHistory;
    }
    
    return destInfo.changeHistory.mid(0, maxRecords);
}

QList<FileChangeRecord> BackupFileMonitor::getRecentChanges(int minutes) const
{
    QList<FileChangeRecord> recentChanges;
    QDateTime threshold = QDateTime::currentDateTime().addSecs(-minutes * 60);
    
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        const DestinationMonitorInfo &destInfo = it.value();
        
        for (const FileChangeRecord &change : destInfo.changeHistory) {
            if (change.changeTime >= threshold) {
                recentChanges.append(change);
            }
        }
    }
    
    return recentChanges;
}

int BackupFileMonitor::getTotalFilesMonitored() const
{
    int total = 0;
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        total += it->fileCount;
    }
    return total;
}

int BackupFileMonitor::getFileCountInDestination(const QString &destinationId) const
{
    if (!m_destinations.contains(destinationId)) {
        return 0;
    }
    return m_destinations[destinationId].fileCount;
}

qint64 BackupFileMonitor::getTotalSizeMonitored() const
{
    qint64 total = 0;
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        total += it->totalSize;
    }
    return total;
}

qint64 BackupFileMonitor::getSizeInDestination(const QString &destinationId) const
{
    if (!m_destinations.contains(destinationId)) {
        return 0;
    }
    return m_destinations[destinationId].totalSize;
}

QDateTime BackupFileMonitor::getLastScanTime(const QString &destinationId) const
{
    if (!m_destinations.contains(destinationId)) {
        return QDateTime();
    }
    return m_destinations[destinationId].lastScan;
}

bool BackupFileMonitor::verifyFileIntegrity(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    // Find which destination this file belongs to
    QString destinationId = findDestinationIdByPath(filePath);
    if (destinationId.isEmpty()) {
        return false;
    }
    
    const DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    if (!destInfo.files.contains(filePath)) {
        return false;
    }
    
    const BackupFileInfo &storedInfo = destInfo.files[filePath];
    
    // Check if file still has the same size and modification date
    if (fileInfo.size() != storedInfo.size ||
        fileInfo.lastModified() != storedInfo.lastModified) {
        return false;
    }
    
    return true;
}

QStringList BackupFileMonitor::findCorruptedFiles(const QString &destinationId)
{
    QStringList corruptedFiles;
    
    if (!m_destinations.contains(destinationId)) {
        return corruptedFiles;
    }
    
    const DestinationMonitorInfo &destInfo = m_destinations[destinationId];
    
    for (auto it = destInfo.files.begin(); it != destInfo.files.end(); ++it) {
        const QString &filePath = it.key();
        
        if (!verifyFileIntegrity(filePath)) {
            corruptedFiles.append(filePath);
            emit corruptedFileFound(filePath, "File integrity check failed");
        }
    }
    
    return corruptedFiles;
}

bool BackupFileMonitor::saveState(const QString &filePath)
{
    QJsonObject root;
    root["version"] = "1.0";
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["monitoring_enabled"] = m_monitoringEnabled;
    root["scan_interval"] = m_scanIntervalMinutes;
    
    QJsonArray destinationsArray;
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        const DestinationMonitorInfo &destInfo = it.value();
        
        QJsonObject destObj;
        destObj["destination_id"] = destInfo.destinationId;
        destObj["path"] = destInfo.path;
        destObj["file_count"] = destInfo.fileCount;
        destObj["total_size"] = QString::number(destInfo.totalSize);
        destObj["last_scan"] = destInfo.lastScan.toString(Qt::ISODate);
        
        // Save file list
        QJsonArray filesArray;
        for (auto fileIt = destInfo.files.begin(); fileIt != destInfo.files.end(); ++fileIt) {
            const BackupFileInfo &fileInfo = fileIt.value();
            
            QJsonObject fileObj;
            fileObj["path"] = fileInfo.filePath;
            fileObj["name"] = fileInfo.fileName;
            fileObj["size"] = QString::number(fileInfo.size);
            fileObj["last_modified"] = fileInfo.lastModified.toString(Qt::ISODate);
            fileObj["last_checked"] = fileInfo.lastChecked.toString(Qt::ISODate);
            
            filesArray.append(fileObj);
        }
        destObj["files"] = filesArray;
        
        destinationsArray.append(destObj);
    }
    root["destinations"] = destinationsArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error(QString("Failed to save state: %1").arg(file.errorString()));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool BackupFileMonitor::loadState(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;  // File doesn't exist or can't be opened
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        emit error("Invalid state file format");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Restore settings
    m_monitoringEnabled = root["monitoring_enabled"].toBool();
    m_scanIntervalMinutes = root["scan_interval"].toInt(30);
    
    // Restore destinations
    QJsonArray destinationsArray = root["destinations"].toArray();
    for (const QJsonValue &destValue : destinationsArray) {
        QJsonObject destObj = destValue.toObject();
        
        QString destinationId = destObj["destination_id"].toString();
        QString path = destObj["path"].toString();
        
        // Check if path still exists
        if (!QDir(path).exists()) {
            continue;
        }
        
        DestinationMonitorInfo destInfo;
        destInfo.destinationId = destinationId;
        destInfo.path = path;
        destInfo.fileCount = destObj["file_count"].toInt();
        destInfo.totalSize = destObj["total_size"].toString().toLongLong();
        destInfo.lastScan = QDateTime::fromString(destObj["last_scan"].toString(), Qt::ISODate);
        
        // Restore file list
        QJsonArray filesArray = destObj["files"].toArray();
        for (const QJsonValue &fileValue : filesArray) {
            QJsonObject fileObj = fileValue.toObject();
            
            BackupFileInfo fileInfo;
            fileInfo.filePath = fileObj["path"].toString();
            fileInfo.fileName = fileObj["name"].toString();
            fileInfo.size = fileObj["size"].toString().toLongLong();
            fileInfo.lastModified = QDateTime::fromString(fileObj["last_modified"].toString(), Qt::ISODate);
            fileInfo.lastChecked = QDateTime::fromString(fileObj["last_checked"].toString(), Qt::ISODate);
            fileInfo.isValid = QFileInfo::exists(fileInfo.filePath);
            
            destInfo.files[fileInfo.filePath] = fileInfo;
        }
        
        m_destinations[destinationId] = destInfo;
        m_pathToDestinationMap[path] = destinationId;
        
        // Start watching
        startWatching(path);
    }
    
    // Start timer if monitoring was enabled
    if (m_monitoringEnabled) {
        m_scanTimer->start();
    }
    
    return true;
}

QString BackupFileMonitor::calculateChecksum(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return QString(hash.result().toHex());
    }
    
    return QString();
}

bool BackupFileMonitor::isBackupFile(const QString &fileName) const
{
    // Filter backup files by extension or pattern
    // Customize this based on your backup file naming convention
    QString lower = fileName.toLower();
    
    return lower.endsWith(".zip") ||
           lower.endsWith(".7z") ||
           lower.endsWith(".tar.gz") ||
           lower.endsWith(".tar") ||
           lower.endsWith(".bak") ||
           lower.endsWith(".backup") ||
           lower.contains("backup");
}

QString BackupFileMonitor::findDestinationIdByPath(const QString &path) const
{
    // Try exact match first
    for (auto it = m_destinations.begin(); it != m_destinations.end(); ++it) {
        if (path.startsWith(it->path)) {
            return it.key();
        }
    }
    
    return QString();
}

void BackupFileMonitor::startWatching(const QString &path)
{
    if (!m_fileWatcher->directories().contains(path)) {
        m_fileWatcher->addPath(path);
    }
}

void BackupFileMonitor::stopWatching(const QString &path)
{
    if (m_fileWatcher->directories().contains(path)) {
        m_fileWatcher->removePath(path);
    }
}

void BackupFileMonitor::onDirectoryChanged(const QString &path)
{
    QString destinationId = m_pathToDestinationMap.value(path);
    if (destinationId.isEmpty()) {
        // Try to find by prefix
        destinationId = findDestinationIdByPath(path);
    }
    
    if (!destinationId.isEmpty() && m_monitoringEnabled) {
        // Schedule a scan
        QTimer::singleShot(1000, this, [this, destinationId]() {
            scanDestination(destinationId);
        });
    }
}

void BackupFileMonitor::onFileChanged(const QString &path)
{
    QString destinationId = findDestinationIdByPath(path);
    
    if (!destinationId.isEmpty() && m_monitoringEnabled) {
        // Schedule a scan
        QTimer::singleShot(1000, this, [this, destinationId]() {
            scanDestination(destinationId);
        });
    }
}

void BackupFileMonitor::onScanTimerTimeout()
{
    if (m_monitoringEnabled) {
        scanAllDestinations();
    }
}
