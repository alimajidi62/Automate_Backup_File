# Automated Backup File

A cross-platform automated backup solution that monitors network and cloud locations, detects file changes, and creates encrypted backups.

## Overview

https://github.com/user-attachments/assets/710cf241-e046-4a3d-81c8-cf92a8e18d7b

🚧 **In Development** - Qt/C++ implementation with comprehensive backend architecture

### ✅ Completed Features
- **5-Tab UI**: Sources, Schedule, Tasks, Destinations, Settings with custom QSS styling
- **Source Management**: Local/network/cloud sources with authentication, connectivity testing, and file system monitoring
- **Destination Management**: Multiple destinations with retention policies and space tracking
- **File Monitoring**: Real-time change detection (Added/Modified/Deleted/Renamed) with configurable intervals
- **Cloud Framework**: Abstract CloudProvider interface with GoogleDrive, Dropbox, OneDrive, S3 implementations
- **Persistence**: JSON-based configuration storage
- **Cross-Platform Build**: CMake with Qt 6.9.1 (Windows MSVC, Linux GCC, macOS Clang)

### 🔄 In Progress
- Cloud OAuth implementation, backup execution engine, compression/encryption, scheduling

## Key Features
- **Change Detection**: Real-time file monitoring with modification tracking across local/network/cloud sources
- **Scheduled Backups**: Configurable schedules (daily/weekly/monthly) with background execution
- **Multi-Source Support**: Multiple network paths and cloud providers (Google Drive, OneDrive, Dropbox, S3)
- **Authentication**: Secure credential storage with OAuth for cloud services
- **Compression & Encryption**: AES-256 encryption and configurable compression
- **Flexible Destinations**: Local, network, or cloud storage with retention policies
- **Cross-Platform**: Windows, Linux, macOS support

## Technology Stack
- **Framework**: Qt 6.9.1 (Widgets, Concurrent, Network)
- **Build**: CMake 3.16+, C++17
- **Compilers**: MSVC 2022 (Windows), GCC/Clang (Linux/macOS)
- **Platform**: Windows (mpr, netapi32 for network auth)
- **Data**: JSON persistence, QFileSystemWatcher for monitoring

## Project Structure
```
QT/AutomatedBackupFile/
├── UI Components: mainwindow, sourcestab, scheduletab, taskstab, destinationtab, settingstab (.cpp/.h/.ui)
├── Dialogs: networkcredentialsdialog, cloudauthdialog
├── Managers: sourcemanager, destinationmanager
├── Models: backupsource, backupdestination, retentionpolicy, cloudprovider, backupfilemonitor
├── Resources: resources.qrc, styles.qss
└── Build: CMakeLists.txt, build/
```

## Installation & Building

### Prerequisites
- Qt 6.9.1+, CMake 3.16+, C++17 compiler (MSVC 2022/GCC 9+/Clang 10+)

### Build
```powershell
# Windows
cd QT/AutomatedBackupFile/build
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.9.1\msvc2022_64"
cmake .. ; cmake --build . --config Debug
.\Debug\AutomatedBackupFile.exe

# Linux/macOS
cd QT/AutomatedBackupFile
mkdir build && cd build
cmake .. && cmake --build .
./AutomatedBackupFile
```

## Usage

The application features a 5-tab interface:

1. **Backup Sources**: Add local/network/cloud sources with authentication, test connectivity, enable file monitoring
2. **Schedule**: Configure automated schedules (UI ready, backend pending)
3. **Backup Tasks**: Monitor active backups with progress bars (UI ready, backend pending)
4. **Destinations**: Add destinations, configure retention policies, enable file monitoring with change history
5. **Settings**: Configure compression/encryption/notifications (UI ready, backend pending)

## Roadmap

### Phase 1: UI & Architecture ✅ (Completed)
- [x] Qt UI framework with modular architecture
- [x] Tab-based interface (Sources, Schedule, Tasks, Destination, Settings)
- [x] Modular widget system (.cpp/.h/.ui separation)
- [x] Custom QSS styling
- [x] BackupSource data model with JSON serialization
- [x] BackupDestination data model
- [x] SourceManager backend class
- [x] DestinationManager backend class
- [x] RetentionPolicy class
- [x] CloudProvider abstract interface
- [x] Network authentication dialog
- [x] Cloud authentication dialog structure

### Phase 2: Source & Destination Management ✅ (Completed)
- [x] Local source management with file browser
- [x] Network source management with UNC paths
- [x] Network credential authentication (username/password/domain)
- [x] Source connectivity testing
- [x] File system watcher integration
- [x] Change monitoring configuration
- [x] Source status tracking and display
- [x] JSON persistence for sources
- [x] Local/network/cloud destination support
- [x] Retention policy configuration
- [x] Space monitoring per destination
- [x] JSON persistence for destinations
- [x] BackupFileMonitor class for destination file tracking
- [x] Real-time change detection in destinations
- [x] Change history with timestamps
- [x] File integrity verification
- [x] Periodic scanning with configurable intervals
- [x] Monitoring state persistence

### Phase 3: Core Backup Functionality (In Progress)
- [x] File system monitoring framework
- [x] Change detection structure
- [x] Network path validation
- [ ] Backup task execution engine
- [ ] File copy operations with progress tracking
- [ ] Multi-threaded backup operations
- [ ] Backup queue management
- [ ] Error handling and retry logic
- [ ] Progress reporting to UI

### Phase 4: Cloud & Compression (Next)
- [x] Cloud provider architecture
- [x] Provider factory pattern
- [ ] OAuth implementation (Google Drive, Dropbox, OneDrive)
- [ ] Amazon S3 authentication and operations
- [ ] Cloud upload/download with progress
- [ ] Compression engine (ZIP, 7Z)
- [ ] Compression level configuration
- [ ] Encrypted compression format

### Phase 5: Security & Encryption
- [ ] AES-256 encryption implementation
- [ ] Secure credential storage (encrypted vault)
- [ ] Password-protected backups
- [ ] Integrity verification (SHA-256 checksums)
- [ ] Certificate-based authentication for cloud
- [ ] Two-factor authentication support

### Phase 6: Scheduling & Automation
- [ ] Schedule execution engine
- [ ] Cron-like schedule syntax
- [ ] Background service/daemon mode
- [ ] Windows service integration
- [ ] Linux systemd service
- [ ] macOS LaunchAgent
- [ ] Wake system for scheduled backups

### Phase 7: Advanced Features
- [ ] Incremental backup support
- [ ] Differential backup support
- [ ] Backup restoration functionality
- [ ] Point-in-time recovery
- [ ] Backup verification
- [ ] Email/SMS notifications
- [ ] Webhook notifications
- [ ] Comprehensive logging system
- [ ] Log rotation and cleanup

### Phase 8: Enterprise & Polish
- [ ] Command-line interface
- [ ] REST API for remote management
- [ ] Backup profiles and templates
- [ ] Configuration import/export
- [ ] Multi-language support (i18n)
- [ ] Dark/light theme toggle
- [ ] Database storage option (SQLite)
- [ ] Performance profiling and optimization
- [ ] Memory usage optimization
- [ ] Cross-platform testing suite
- [ ] Documentation and user manual
- [ ] Installer packages (MSI, DEB, DMG)

## Architecture

**3-Layer Design Pattern:**
- **UI Layer**: Tab widgets (.cpp/.h/.ui) - MainWindow, SourcesTab, ScheduleTab, TasksTab, DestinationTab, SettingsTab, Dialogs
- **Business Logic**: Manager classes - SourceManager, DestinationManager, CloudProviderFactory
- **Data Models**: BackupSource, BackupDestination, RetentionPolicy, CloudProvider, BackupFileMonitor

**Key Patterns**: Factory (CloudProvider), Manager (Source/Destination), Model-View (separation), Observer (Qt signals/slots), Strategy (cloud implementations)

## Roadmap

**Phase 1-2**: ✅ UI, source/destination management, file monitoring  
**Phase 3**: 🔄 Backup execution engine, progress tracking  
**Phase 4**: Cloud OAuth, compression (ZIP/7Z), encryption (AES-256)  
**Phase 5**: Schedule execution, background service, logging  
**Phase 6**: Restoration, incremental backups, notifications, CLI

## Contributing

Contributions welcome! Follow Qt conventions, maintain UI/logic/data separation, use Qt signals/slots, add tests for business logic.

## Contributing

Contributions welcome! Follow Qt conventions, maintain UI/logic/data separation, use Qt signals/slots, add tests for business logic.

## License

*To be determined*

## Contact

**Repository**: [github.com/alimajidi62/Automate_Backup_File](https://github.com/alimajidi62/Automate_Backup_File)  
**Author**: Ali Majidi (alimajidi62)

---
*Project under active development - features and documentation updated regularly*
I would like to add c# wpf to this repo