# Automated Backup File

A cross-platform automated backup solution that monitors network and cloud locations, detects file changes, and creates encrypted backups to local or cloud destinations.

## Overview

https://github.com/user-attachments/assets/710cf241-e046-4a3d-81c8-cf92a8e18d7b

This project provides an intelligent backup system that automatically tracks file changes across network shares and cloud storage, compresses and encrypts the data, and stores it securely in your chosen destination.

## Current Status

🚧 **In Development** - This project currently focuses on the Qt (C++) implementation with a comprehensive backend architecture. Active development is ongoing.

### ✅ Completed

#### User Interface
- **Qt UI Framework**: Fully functional modular UI with 5 main tabs
  - **Sources Tab**: Add/edit/remove local, network, and cloud sources with authentication
  - **Schedule Tab**: Configure backup schedules (UI ready, backend pending)
  - **Tasks Tab**: Monitor backup operations with progress tracking
  - **Destination Tab**: Manage backup destinations with retention policies
  - **Settings Tab**: Configure compression, encryption, and notifications
- **Custom Styling**: Professional QSS stylesheet for consistent UI appearance
- **Responsive Design**: Resizable windows with proper layout management

#### Backend Architecture
- **Source Management System**:
  - `BackupSource` class: Represents local/network/cloud sources with metadata
  - `SourceManager`: Handles source CRUD operations, connectivity testing, and file monitoring
  - File system watcher integration for change detection
  - JSON-based persistence for sources configuration
  - Network authentication support (username/password/domain)
  - Source status tracking (Available, Unavailable, Checking, Error, CredentialsRequired)

- **Destination Management System**:
  - `BackupDestination` class: Represents local/network/cloud destinations
  - `DestinationManager`: Manages destinations, space monitoring, and retention policies
  - `RetentionPolicy` class: Configurable retention with daily/weekly/monthly backup preservation
  - `BackupFileMonitor` class: Monitors backup files in destinations with change detection
  - File system watcher integration for real-time change notifications
  - Periodic scanning with configurable intervals
  - Change history tracking (Added, Modified, Deleted, Renamed, Size Changed)
  - File integrity verification and corrupted file detection
  - Space availability tracking and best-destination selection algorithm
  - JSON-based persistence for destinations configuration and monitoring state

- **Cloud Integration Framework**:
  - `CloudProvider` base class with virtual interface
  - Provider implementations: GoogleDrive, Dropbox, OneDrive, AmazonS3
  - OAuth authentication framework (structure in place)
  - Network-based operations using Qt's QNetworkAccessManager
  - Mock provider for testing without actual cloud credentials
  - Factory pattern for provider creation

- **Authentication & Security**:
  - `NetworkCredentialsDialog`: GUI for entering network credentials
  - `CloudAuthDialog`: GUI for OAuth-based cloud authentication
  - Credential storage in backend models (encryption pending)
  - Windows networking library integration (mpr, netapi32)

- **Build System**:
  - CMake configuration with Qt 6.9.1
  - AutoUIC, AutoMOC, AutoRCC enabled
  - Qt Widgets, Concurrent, and Network modules integrated
  - Cross-platform support (Windows MSVC, Linux GCC, macOS Clang)
  - Resource file system (QRC) for stylesheets and assets

### 🔄 In Progress
- Cloud provider API implementation (OAuth flow, upload/download operations)
- Backup task execution engine
- Compression algorithm integration
- Encryption implementation (AES-256)
- Schedule execution system
- Progress reporting and logging

### 🚫 Known Limitations
- File checksums (MD5/SHA256) structure in place but not yet computed
- Cloud destination file monitoring not yet supported (only local/network)
- Change detection relies on modification dates (not content-based)

## Planned Features

### 1. **Change Detection**
- Monitor specified network addresses for file modifications
- Track changes since the last backup operation
- Identify new, modified, and deleted files
- Support for multiple network locations simultaneously

### 2. **Task Management**
- Auto-generate todo lists for upcoming backup processes
- Prioritize files based on modification time and importance
- Display pending backup operations in a clear interface

### 3. **Scheduled Backups**
- Configure flexible backup schedules (daily, weekly, monthly, custom)
- Support for multiple backup profiles with different schedules
- Manual backup trigger option
- Background service/daemon mode for automatic execution

### 4. **Multiple Source Support**
- Accept multiple network paths from user input
- Support for cloud storage providers (planned: Google Drive, OneDrive, Dropbox, AWS S3)
- Monitor both network shares (SMB/CIFS) and cloud directories
- Batch processing of multiple locations

### 5. **Authentication Management**
- Secure credential storage for network access
- Support for username/password authentication
- Cloud service OAuth integration
- Encrypted credential vault

### 6. **Compression & Encryption**
- File compression to reduce backup size
- AES-256 encryption for all backup archives
- Password-protected backup files
- Integrity verification (checksums/hashes)

### 7. **Flexible Destination Options**
- Local storage (HDD, SSD, external drives)
- Network locations
- Cloud storage services
- Configurable retention policies

### 8. **Cross-Platform Support**
- Windows support (WPF and Qt implementations)
- Linux support (Qt implementation)
- macOS support (Qt implementation)
- Consistent functionality across all platforms

## Technology Stack

### Qt Implementation (C++)
- **Framework**: Qt 6.9.1
- **Build System**: CMake 3.16+
- **Compiler**: MSVC 2022 (Windows), GCC/Clang (Linux/macOS)
- **Qt Modules**: 
  - QtWidgets (UI components)
  - QtConcurrent (Multi-threading)
  - QtNetwork (HTTP requests, cloud integration)
- **C++ Standard**: C++17
- **Platform Libraries**: Windows Networking (mpr, netapi32) for network authentication

### Architecture Components
- **UI Layer**: Modular tab-based widgets with `.ui` files
- **Business Logic**: Manager classes (SourceManager, DestinationManager)
- **Data Models**: BackupSource, BackupDestination, RetentionPolicy, CloudProvider
- **Persistence**: JSON-based configuration storage
- **Styling**: QSS (Qt Style Sheets) for custom appearance

## Project Structure

```
Automate_Backup_File/
├── README.md
├── QT/AutomatedBackupFile/         # Qt/C++ implementation
│   ├── main.cpp                    # Application entry point
│   │
│   ├── UI Components (Tab Widgets)
│   ├── mainwindow.cpp/h/ui         # Main window (tab container)
│   ├── sourcestab.cpp/h/ui         # Backup sources management UI
│   ├── scheduletab.cpp/h/ui        # Schedule configuration UI
│   ├── taskstab.cpp/h/ui           # Backup operations & tasks UI
│   ├── destinationtab.cpp/h/ui     # Destination settings UI
│   ├── settingstab.cpp/h/ui        # App settings & encryption UI
│   │
│   ├── Dialogs
│   ├── networkcredentialsdialog.cpp/h  # Network authentication dialog
│   ├── cloudauthdialog.cpp/h           # Cloud OAuth dialog
│   │
│   ├── Backend Logic (Manager Classes)
│   ├── sourcemanager.cpp/h         # Source CRUD operations & monitoring
│   ├── destinationmanager.cpp/h    # Destination management & space tracking
│   │
│   ├── Data Models
│   ├── backupsource.cpp/h          # Source data model with JSON serialization
│   ├── backupdestination.cpp/h     # Destination data model
│   ├── retentionpolicy.cpp/h       # Retention policy configuration
│   ├── cloudprovider.cpp/h         # Cloud provider interface & implementations
│   ├── backupfilemonitor.cpp/h     # File monitoring for destinations (NEW!)
│   │
│   ├── Resources
│   ├── resources.qrc               # Qt resource file
│   ├── styles.qss                  # Custom stylesheet
│   ├── BACKUPFILEMONITOR_USAGE.md  # File monitor documentation (NEW!)
│   │
│   ├── Build System
│   ├── CMakeLists.txt              # Build configuration
│   └── build/                      # Build output directory
│       ├── AutomatedBackupFile.sln # Visual Studio solution
│       └── Debug/                  # Debug build artifacts
│
└── build/                          # Legacy build directory
```

## Installation

### Prerequisites

#### For Qt Version:
- Qt 6.9.1 or higher
- CMake 3.16 or higher
- C++17 compatible compiler
- Windows: Visual Studio 2022 or higher with MSVC compiler
- Linux: GCC 9+ or Clang 10+
- macOS: Xcode 12+

### Building from Source

#### Qt Version (Windows with MSVC):
```powershell
cd QT/AutomatedBackupFile
mkdir build
cd build

# Set Qt path (adjust path to your Qt installation)
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.9.1\msvc2022_64"

# Configure and build
cmake ..
cmake --build . --config Debug

# Or open in Visual Studio
# build\AutomatedBackupFile.sln
```

#### Qt Version (Linux/macOS):
```bash
cd QT/AutomatedBackupFile
mkdir build && cd build

# Linux with GCC
cmake ..
cmake --build .

# macOS with Clang
cmake ..
cmake --build .
```

## Usage

### Running the Application

#### Qt Version:
After building, run the executable:
```powershell
# Windows
.\build\Debug\AutomatedBackupFile.exe

# Linux/macOS
./build/AutomatedBackupFile
```

### UI Overview

The application features a tabbed interface with the following sections:

1. **Backup Sources Tab** (Fully Functional):
   - Add local directories via folder browser
   - Add network paths with UNC path support (\\\\server\\share)
   - Network authentication with username/password/domain
   - Test connectivity for each source
   - Edit/remove sources
   - View source status with color-coded indicators
   - Enable file system change monitoring
   - Configure check interval for monitoring
   - Auto-save/load sources configuration

2. **Schedule Tab** (UI Ready):
   - Configure automated backup schedules
   - Support for daily, weekly, monthly, custom intervals
   - Enable/disable scheduler
   - *Backend implementation pending*

3. **Backup Tasks Tab** (UI Ready):
   - Monitor active backup operations
   - View backup progress with progress bars
   - Check backup history and logs
   - Start/stop manual backups
   - *Backend implementation pending*

4. **Destination Tab** (Fully Functional):
   - Add local, network, and cloud destinations
   - Configure retention policies (days, max count, size limits)
   - Enable daily/weekly/monthly backup preservation
   - Monitor available space per destination
   - Support for multiple cloud providers
   - Auto-cleanup based on retention rules
   - **File monitoring with change detection** (NEW!)
   - Real-time tracking of backup files
   - Change history with timestamps
   - Detect added, modified, deleted, renamed files
   - File integrity verification
   - Configurable scan intervals
   - Automatic state persistence

5. **Settings Tab** (UI Ready):
   - Configure compression settings
   - Set encryption options (AES-256)
   - Notification preferences
   - *Backend implementation pending*

### Backend Features

#### Source Management
- Automatic source validation and connectivity testing
- Real-time status monitoring (Available/Unavailable/Checking/Error)
- File system watcher integration for change detection
- Network path authentication with Windows credential integration
- Source statistics (file count, total size)
- JSON-based configuration persistence

#### Destination Management
- Multi-destination support (local, network, cloud)
- Space availability tracking
- Intelligent destination selection based on available space
- Retention policy enforcement
- Cloud provider factory pattern for extensibility
- **Real-time file monitoring** (NEW!)
  - QFileSystemWatcher integration for immediate change detection
  - Periodic scanning with configurable intervals (default: 30 minutes)
  - Comprehensive change tracking (Added, Modified, Deleted, Renamed, Size Changed)
  - Change history per destination (up to 1000 records)
  - File metadata tracking (size, modification date, checksum structure)
  - File integrity verification
  - Corrupted file detection
  - JSON persistence for monitoring state
  - Statistics: file count, total size, last scan time per destination

#### Cloud Provider Framework
- Abstract CloudProvider base class
- Implemented providers: GoogleDrive, Dropbox, OneDrive, Amazon S3
- Mock provider for testing
- OAuth authentication structure
- Upload/download operations interface
- Space quota management

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

### Modular Design Pattern

The Qt implementation follows a clean architecture with clear separation between UI and business logic:

#### UI Layer (View)
Each tab is a separate widget with three files:
- **MainWindow** (.cpp/.h/.ui): Main container with tab widget and status bar
- **SourcesTab** (.cpp/.h/.ui): Source management interface
- **ScheduleTab** (.cpp/.h/.ui): Backup scheduling interface
- **TasksTab** (.cpp/.h/.ui): Task monitoring interface
- **DestinationTab** (.cpp/.h/.ui): Destination configuration interface
- **SettingsTab** (.cpp/.h/.ui): Application settings interface

**Dialogs**:
- **NetworkCredentialsDialog**: Collect network authentication (username/password/domain)
- **CloudAuthDialog**: Handle OAuth flow for cloud providers

#### Business Logic Layer (Controller/Manager)
Manager classes handle all business operations:
- **SourceManager**: 
  - CRUD operations for sources
  - Connectivity testing (local, network, cloud)
  - File system monitoring using QFileSystemWatcher
  - Change detection and notification
  - JSON persistence

- **DestinationManager**:
  - CRUD operations for destinations
  - Space monitoring and availability checking
  - Retention policy enforcement
  - Best destination selection algorithm
  - Cloud provider integration
  - JSON persistence

- **CloudProviderFactory**:
  - Factory pattern for creating cloud provider instances
  - Provider registration and discovery

#### Data Model Layer (Model)
Clean data models with business logic:
- **BackupSource**:
  - Properties: path, type, status, credentials, statistics
  - Methods: validation, serialization (toJson/fromJson), display formatting
  - Enums: SourceType (Local/Network/Cloud), SourceStatus

- **BackupDestination**:
  - Properties: path, type, status, space info, credentials
  - Methods: validation, space calculation, serialization
  - Enums: DestinationType, DestinationStatus

- **RetentionPolicy**:
  - Properties: retention days, max count, max size, preservation rules
  - Methods: shouldDeleteBackup(), policy description
  - Supports daily/weekly/monthly backup preservation

- **CloudProvider** (Abstract):
  - Virtual interface: authenticate, uploadFile, downloadFile, deleteFile, listFiles
  - Concrete implementations: GoogleDriveProvider, DropboxProvider, OneDriveProvider, AmazonS3Provider
  - Mock implementation for testing

### Design Benefits
- **Separation of Concerns**: UI, logic, and data are clearly separated
- **Testability**: Business logic can be tested independently of UI
- **Maintainability**: Easy to modify one layer without affecting others
- **Extensibility**: New cloud providers can be added by implementing CloudProvider interface
- **Reusability**: Manager classes can be reused in CLI or service modes
- **Type Safety**: Strong typing with enums and classes
- **Signal/Slot Architecture**: Qt's event-driven model for loose coupling

### Key Patterns Used
- **Factory Pattern**: CloudProviderFactory for provider instantiation
- **Manager Pattern**: Centralized management of sources and destinations
- **Model-View Pattern**: Data models separate from UI representation
- **Observer Pattern**: Qt signals/slots for event notification
- **Strategy Pattern**: Different cloud provider implementations
- **Singleton-like**: Manager instances owned by tab widgets

## Key Features Summary

### ✅ Currently Working
1. **Local Source Management**: Browse and add local directories
2. **Network Source Management**: Add UNC paths with authentication
3. **Source Authentication**: Username/password/domain for network paths
4. **Connectivity Testing**: Test each source for availability
5. **Source Monitoring**: File system watcher with configurable check intervals
6. **Status Tracking**: Real-time status updates with color-coded display
7. **Source Persistence**: Auto-save/load sources from JSON
8. **Destination Management**: Add multiple destinations (local/network/cloud)
9. **Retention Policies**: Configure automatic cleanup rules
10. **Space Monitoring**: Track available space per destination
11. **Cloud Framework**: Extensible cloud provider architecture
12. **Professional UI**: Custom styled interface with QSS
13. **Cross-platform Build**: CMake configuration for Windows/Linux/macOS
14. **Destination File Monitoring**: Real-time tracking of backup files in destinations
15. **Change Detection**: Track file additions, modifications, deletions, renames
16. **Change History**: Maintain detailed history with timestamps
17. **File Integrity**: Verify file integrity and detect corrupted files
18. **Monitoring State Persistence**: Save/load monitoring state across sessions

### 🔨 Next to Implement
1. Cloud provider OAuth implementation
2. Backup task execution engine
3. File compression (ZIP/7Z)
4. AES-256 encryption
5. Schedule execution
6. Progress tracking UI updates
7. Logging system
8. Backup restoration

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

### Development Guidelines
- Follow Qt coding conventions for C++ code
- Maintain separation between UI (tabs), logic (managers), and data (models)
- Use Qt's signal/slot mechanism for communication between components
- Add Doxygen-style comments for public methods
- Write unit tests for business logic (managers and models)
- Update documentation when adding features
- Test on multiple platforms when possible
- Use Qt Creator or Visual Studio for development
- Ensure CMake configuration works on all platforms

### Code Organization
- UI files: `*tab.cpp/h/ui` and `*dialog.cpp/h/ui`
- Manager classes: `*manager.cpp/h`
- Data models: `backup*.cpp/h`, `retention*.cpp/h`, `cloud*.cpp/h`
- Keep `.cpp` files focused (< 500 lines preferred)
- Use forward declarations in headers to reduce compile dependencies

## Security Considerations

### Current Implementation
- Network credentials are stored in memory during session
- Credential dialogs collect username/password/domain securely
- Windows networking library integration for authenticated network access

### Planned Security Features
- AES-256 encryption for all backup archives
- Secure credential vault (encrypted storage)
- Password-protected backup files
- SHA-256 integrity verification for backups
- OAuth 2.0 for cloud provider authentication
- Certificate-based authentication support
- No plain-text credential storage
- Secure memory handling for sensitive data
- Optional two-factor authentication for cloud providers

## License

*License to be determined*

## Known Limitations & Future Work

### Current Limitations
- Cloud provider OAuth flow is structure-only (not yet functional)
- Backup task execution not yet implemented
- Compression and encryption pending implementation
- Schedule execution requires implementation
- Credential storage is in-memory only (not persisted)
- No backup restoration functionality yet
- Limited error recovery in network operations

### Upcoming Milestones
1. **v0.2**: Backup task execution with progress tracking
2. **v0.3**: Compression (ZIP) and basic encryption
3. **v0.4**: Cloud provider OAuth and file operations
4. **v0.5**: Schedule execution and automation
5. **v1.0**: Full feature completion with restoration

## Contact

**Repository**: [https://github.com/alimajidi62/Automate_Backup_File](https://github.com/alimajidi62/Automate_Backup_File)

**Author**: Ali Majidi (alimajidi62)

## Acknowledgments

*To be added*

---

**Note**: This project is under active development. Features and documentation will be updated regularly as development progresses.
