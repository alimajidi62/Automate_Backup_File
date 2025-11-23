# BackupFileMonitor Usage Guide

## Overview

The `BackupFileMonitor` class provides comprehensive file system monitoring for backup destinations. It tracks file additions, modifications, deletions, and maintains a complete change history with timestamps.

## Features

- **Real-time Monitoring**: Uses Qt's QFileSystemWatcher for immediate change detection
- **Periodic Scanning**: Configurable interval scanning to catch missed changes
- **Change Detection**: Tracks Added, Modified, Deleted, Renamed, and Size Changed events
- **Change History**: Maintains detailed history of all file changes per destination
- **File Statistics**: Tracks file counts, sizes, and last scan times
- **Persistence**: Save/load monitoring state to JSON for session continuity
- **Integrity Verification**: Verify file integrity based on size and modification date
- **Corrupted File Detection**: Find files that don't match stored metadata

## Quick Start

```cpp
// Create monitor instance
BackupFileMonitor *monitor = new BackupFileMonitor(this);

// Add destinations to monitor
monitor->addDestinationPath("dest-001", "C:/Backups/Local");
monitor->addDestinationPath("dest-002", "D:/Backups/Archive");

// Configure monitoring
monitor->setScanInterval(30);  // Scan every 30 minutes
monitor->setMonitoringEnabled(true);

// Connect signals to receive notifications
connect(monitor, &BackupFileMonitor::fileAdded,
        this, &MyClass::onFileAdded);
connect(monitor, &BackupFileMonitor::fileModified,
        this, &MyClass::onFileModified);
connect(monitor, &BackupFileMonitor::fileDeleted,
        this, &MyClass::onFileDeleted);
```

## Integration with DestinationTab

The `DestinationTab` now automatically integrates `BackupFileMonitor`:

```cpp
// In DestinationTab constructor
m_backupFileMonitor = new BackupFileMonitor(this);

// When adding a destination
if (m_destinationManager->addDestination(destination)) {
    m_backupFileMonitor->addDestinationPath(destination->getId(), path);
}

// When removing a destination
m_backupFileMonitor->removeDestinationPath(destinationId);
m_destinationManager->removeDestination(destinationId);
```

## Monitoring Operations

### Start/Stop Monitoring
```cpp
// Enable monitoring (starts periodic scans and file watching)
monitor->setMonitoringEnabled(true);

// Disable monitoring
monitor->setMonitoringEnabled(false);

// Check status
bool isActive = monitor->isMonitoringEnabled();
```

### Configure Scan Interval
```cpp
// Set scan interval (in minutes)
monitor->setScanInterval(15);  // Every 15 minutes
monitor->setScanInterval(60);  // Every hour

// Get current interval
int interval = monitor->getScanInterval();
```

### Manual Scanning
```cpp
// Scan specific destination
monitor->scanDestination("dest-001");

// Scan all destinations
monitor->scanAllDestinations();

// Force complete rescan (clears cached data)
monitor->forceRescan();
```

## Retrieving Information

### Get Files in Destination
```cpp
QList<BackupFileInfo> files = monitor->getFilesInDestination("dest-001");

for (const BackupFileInfo &file : files) {
    qDebug() << "File:" << file.fileName;
    qDebug() << "Path:" << file.filePath;
    qDebug() << "Size:" << file.size;
    qDebug() << "Modified:" << file.lastModified;
}
```

### Get Change History
```cpp
// Get last 50 changes for a destination
QList<FileChangeRecord> changes = monitor->getChangeHistory("dest-001", 50);

for (const FileChangeRecord &change : changes) {
    QString type;
    switch (change.changeType) {
        case FileChangeRecord::Added: type = "Added"; break;
        case FileChangeRecord::Modified: type = "Modified"; break;
        case FileChangeRecord::Deleted: type = "Deleted"; break;
        case FileChangeRecord::Renamed: type = "Renamed"; break;
        case FileChangeRecord::SizeChanged: type = "Size Changed"; break;
    }
    
    qDebug() << change.changeTime.toString() 
             << type << change.description;
}

// Get recent changes across all destinations (last 60 minutes)
QList<FileChangeRecord> recent = monitor->getRecentChanges(60);
```

### Statistics
```cpp
// Total statistics
int totalFiles = monitor->getTotalFilesMonitored();
qint64 totalSize = monitor->getTotalSizeMonitored();

// Per-destination statistics
int destFiles = monitor->getFilesInDestination("dest-001");
qint64 destSize = monitor->getSizeInDestination("dest-001");
QDateTime lastScan = monitor->getLastScanTime("dest-001");
```

## File Integrity

### Verify File Integrity
```cpp
QString filePath = "C:/Backups/Local/backup_2025-11-23.zip";

if (monitor->verifyFileIntegrity(filePath)) {
    qDebug() << "File integrity OK";
} else {
    qDebug() << "File may be corrupted or modified";
}
```

### Find Corrupted Files
```cpp
QStringList corrupted = monitor->findCorruptedFiles("dest-001");

if (!corrupted.isEmpty()) {
    qDebug() << "Corrupted files found:";
    for (const QString &file : corrupted) {
        qDebug() << " -" << file;
    }
}
```

## Persistence

### Save State
```cpp
// Save monitoring state to JSON file
if (monitor->saveState("file_monitor.json")) {
    qDebug() << "State saved successfully";
}
```

### Load State
```cpp
// Load monitoring state from JSON file
if (monitor->loadState("file_monitor.json")) {
    qDebug() << "State loaded successfully";
    // Monitoring continues from saved state
}
```

## Signal Reference

### File Change Signals
- `fileAdded(destinationId, filePath, info)` - New file detected
- `fileModified(destinationId, filePath, oldInfo, newInfo)` - File modified
- `fileDeleted(destinationId, filePath, info)` - File deleted
- `fileRenamed(destinationId, oldPath, newPath)` - File renamed
- `sizeChanged(destinationId, filePath, oldSize, newSize)` - File size changed

### Scan Signals
- `scanStarted(destinationId)` - Scan operation started
- `scanCompleted(destinationId, filesFound, changesDetected)` - Scan finished
- `scanError(destinationId, error)` - Scan failed

### Other Signals
- `changeDetected(destinationId, change)` - Any change detected
- `corruptedFileFound(filePath, reason)` - File integrity issue
- `monitoringStateChanged(enabled)` - Monitoring enabled/disabled
- `error(errorMessage)` - General error occurred

## Backup File Filtering

The monitor automatically filters files based on common backup extensions:
- `.zip`
- `.7z`
- `.tar.gz`
- `.tar`
- `.bak`
- `.backup`
- Files containing "backup" in the name

You can customize this by modifying the `isBackupFile()` method in `backupfilemonitor.cpp`.

## Example: Complete Integration

```cpp
class MyBackupApp : public QWidget
{
    Q_OBJECT
    
public:
    MyBackupApp(QWidget *parent = nullptr) : QWidget(parent)
    {
        // Create monitor
        m_monitor = new BackupFileMonitor(this);
        
        // Load previous state
        m_monitor->loadState("monitor_state.json");
        
        // Setup connections
        connect(m_monitor, &BackupFileMonitor::fileAdded,
                this, &MyBackupApp::onFileAdded);
        connect(m_monitor, &BackupFileMonitor::scanCompleted,
                this, &MyBackupApp::onScanCompleted);
        
        // Configure and start
        m_monitor->setScanInterval(30);
        m_monitor->setMonitoringEnabled(true);
    }
    
    ~MyBackupApp()
    {
        // Save state on exit
        m_monitor->saveState("monitor_state.json");
    }
    
private slots:
    void onFileAdded(const QString &destId, const QString &path, 
                     const BackupFileInfo &info)
    {
        QString msg = QString("New backup detected: %1 (%2 KB)")
            .arg(info.fileName)
            .arg(info.size / 1024);
        showNotification(msg);
    }
    
    void onScanCompleted(const QString &destId, int files, int changes)
    {
        qDebug() << "Destination" << destId 
                 << "has" << files << "files"
                 << "with" << changes << "changes";
        updateUI();
    }
    
private:
    BackupFileMonitor *m_monitor;
};
```

## Best Practices

1. **Always save state on exit**: Call `saveState()` in destructor
2. **Load state on startup**: Call `loadState()` in constructor
3. **Set reasonable scan intervals**: 15-60 minutes for most use cases
4. **Handle signals appropriately**: Update UI or log changes
5. **Check return values**: Validate paths before adding destinations
6. **Clean up properly**: Remove destinations when deleted from manager
7. **Use appropriate file filters**: Customize `isBackupFile()` for your needs

## Performance Considerations

- File system watching is immediate but may miss rapid changes
- Periodic scanning catches missed changes but uses system resources
- Large destinations with many files will take longer to scan
- Change history is limited to 1000 records per destination
- Consider longer intervals for network destinations

## Troubleshooting

### Changes Not Detected
- Ensure monitoring is enabled: `setMonitoringEnabled(true)`
- Check scan interval is reasonable: `setScanInterval(minutes)`
- Verify destination path exists and is accessible
- Check file matches backup file patterns

### High CPU Usage
- Increase scan interval
- Reduce number of monitored destinations
- Check for very large directories with many files

### State Not Persisting
- Verify write permissions for JSON file
- Check file path is absolute
- Ensure `saveState()` is called before exit

## Future Enhancements

- Checksum-based integrity verification (MD5/SHA256)
- Network destination monitoring
- Cloud storage monitoring integration
- Custom file filtering rules
- Email/webhook notifications
- Database storage option
- Compression detection
- Backup validation tools
