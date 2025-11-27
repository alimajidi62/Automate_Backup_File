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
- **Backup Execution Engine**: Multi-threaded backup with copy-encrypt-delete workflow
- **Encryption/Decryption**: Password-based file encryption (XOR + SHA256) with standalone decryption
- **Schedule Management**: Automated backup scheduling with Daily/Weekly/Monthly/Custom intervals
- **Schedule Execution**: Timer-based schedule checking with automatic backup triggering
- **Cloud Framework**: Abstract CloudProvider interface with GoogleDrive, Dropbox, OneDrive, S3 implementations
- **Persistence**: JSON-based configuration storage for sources, destinations, and schedules
- **Cross-Platform Build**: CMake with Qt 6.9.1 (Windows MSVC, Linux GCC, macOS Clang)

### 🔄 In Progress
- Cloud OAuth implementation, compression integration

## Key Features
- **Backup Execution**: Multi-threaded backup engine with progress tracking and cancellation support
- **Encryption**: Password-based file encryption with XOR + SHA256 key derivation
- **Decryption**: Standalone recovery tool to decrypt backups to original files
- **Change Detection**: Real-time file monitoring with modification tracking across local/network/cloud sources
- **Scheduled Backups**: Automated scheduling (Daily/Weekly/Monthly/Custom) with timer-based execution
- **Schedule Types**: 
  - Daily: Runs at specified time every day
  - Weekly: Runs on selected weekdays at specified time
  - Monthly: Runs on specific day of month at specified time
  - Custom: Runs at custom interval (in minutes)
- **Multi-Source Support**: Multiple network paths and cloud providers (Google Drive, OneDrive, Dropbox, S3)
- **Authentication**: Secure credential storage with OAuth for cloud services
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
├── Managers: sourcemanager, destinationmanager, schedulemanager
├── Models: backupsource, backupdestination, backupschedule, retentionpolicy, cloudprovider, backupfilemonitor
├── Backup Engine: backupengine (threading, progress tracking, start/stop control)
├── Encryption: fileencryptor (encrypt files with password), filedecryptor (decrypt backups)
├── Security: key.txt (encryption password storage)
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
2. **Schedule**: Configure automated backup schedules with multiple frequency options
   - Add schedules with Daily/Weekly/Monthly/Custom intervals
   - Quick Schedule form for fast creation
   - Edit/Remove existing schedules
   - Enable/disable automatic scheduler with master checkbox
   - View next run time for each schedule
3. **Backup Tasks**: Start/stop backups, monitor progress with real-time updates, view backup history
4. **Destinations**: Add destinations, configure retention policies, decrypt backups, view change history
5. **Settings**: Configure compression/encryption/notifications, test encryption key

### Backup Workflow
1. Add sources in **Backup Sources** tab (local folders, network shares, or cloud locations)
2. Add destinations in **Destinations** tab (where backups will be stored)
3. **Option A - Manual Backup**: Click **Start Backup Now** in **Backup Tasks** tab
4. **Option B - Scheduled Backup**: Create schedule in **Schedule** tab, enable scheduler, backups run automatically
5. Monitor progress bar and status in real-time
6. Backups are encrypted and stored in `destination/encrypted/` folder
7. To restore: Select destination in **Destinations** tab → Click **Decrypt Backup**
8. Decrypted files appear in `destination/encrypted/decrypted/` folder

### Schedule Configuration
1. Go to **Schedule** tab
2. **Quick Schedule**: Enter name, select frequency, set time → Click **Add Quick Schedule**
3. **Advanced Schedule**: Click **Add Schedule** for full dialog with:
   - Weekly: Select specific weekdays (Mon-Sun)
   - Monthly: Choose day of month (1-31)
   - Custom: Set interval in minutes
4. Enable **"Enable Automatic Backups"** checkbox to start scheduler
5. Schedules run automatically at specified times
6. View next run time in the table
7. Edit/Remove schedules as needed

### Encryption Setup
- Password stored in `key.txt` file (same directory as executable)
- Default password: `123456qwerty` (change before production use)
- Test encryption in **Settings** tab → **Test Encryption**

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

### Phase 3: Core Backup Functionality ✅ (Completed)
- [x] File system monitoring framework
- [x] Change detection structure
- [x] Network path validation
- [x] Backup task execution engine (BackupEngine class)
- [x] Multi-threaded backup operations with QThread
- [x] File copy operations with progress tracking
- [x] Backup workflow: Copy → Encrypt → Delete unencrypted
- [x] Error handling and retry logic
- [x] Progress reporting to UI (progress bar, status label)
- [x] Start/Stop backup control
- [x] File encryption with password (FileEncryptor)
- [x] File decryption for recovery (FileDecryptor)
- [x] Password-based encryption (XOR + SHA256)
- [x] Encrypted file format (.enc extension)
- [x] Standalone decryption in Destinations tab

### Phase 4: Cloud & Compression (Next)
- [x] Cloud provider architecture
- [x] Provider factory pattern
- [ ] OAuth implementation (Google Drive, Dropbox, OneDrive)
- [ ] Amazon S3 authentication and operations
- [ ] Cloud upload/download with progress
- [ ] Compression engine (ZIP, 7Z)
- [ ] Compression level configuration
- [ ] Encrypted compression format

### Phase 5: Security & Encryption (Partially Complete)
- [x] Password-based file encryption (XOR + SHA256)
- [x] Secure key file storage (key.txt)
- [x] Encrypted backup format with .enc extension
- [x] Decryption functionality for recovery
- [ ] AES-256 encryption upgrade
- [ ] Secure credential storage (encrypted vault)
- [ ] Integrity verification (SHA-256 checksums)
- [ ] Certificate-based authentication for cloud
- [ ] Two-factor authentication support

### Phase 6: Scheduling & Automation ✅ (Completed)
- [x] BackupSchedule data model with frequency types
- [x] ScheduleManager backend class with timer execution
- [x] Schedule persistence (schedules.json)
- [x] Daily schedule support
- [x] Weekly schedule with weekday selection
- [x] Monthly schedule with day of month
- [x] Custom interval schedule (minutes)
- [x] Schedule validation and duplicate prevention
- [x] Next run time calculation
- [x] Automatic backup triggering
- [x] Schedule UI integration (add/edit/remove)
- [x] Quick schedule form
- [x] Master enable/disable checkbox
- [x] Timer-based checking (60-second intervals)
- [x] Schedule status display in table
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
- **Business Logic**: Manager classes - SourceManager, DestinationManager, ScheduleManager, BackupEngine, CloudProviderFactory
- **Data Models**: BackupSource, BackupDestination, BackupSchedule, RetentionPolicy, CloudProvider, BackupFileMonitor
- **Security Layer**: FileEncryptor, FileDecryptor with password-based encryption

**Key Patterns**: Factory (CloudProvider), Manager (Source/Destination/Schedule), Model-View (separation), Observer (Qt signals/slots), Strategy (cloud implementations), Worker Thread (BackupEngine), Timer (ScheduleManager)

## Roadmap

**Phase 1-2**: ✅ UI, source/destination management, file monitoring  
**Phase 3**: ✅ Backup execution engine, encryption/decryption, progress tracking  
**Phase 4**: ✅ Schedule management with Daily/Weekly/Monthly/Custom, timer execution  
**Phase 5**: 🔄 Cloud OAuth, compression (ZIP/7Z), AES-256 upgrade  
**Phase 6**: Background service, incremental backups, notifications, CLI  
**Phase 7**: Restoration UI, point-in-time recovery, enterprise features

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