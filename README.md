# Automated Backup File

A cross-platform automated backup solution that monitors network and cloud locations, detects file changes, and creates encrypted backups to local or cloud destinations.

## Overview

https://github.com/user-attachments/assets/710cf241-e046-4a3d-81c8-cf92a8e18d7b

This project provides an intelligent backup system that automatically tracks file changes across network shares and cloud storage, compresses and encrypts the data, and stores it securely in your chosen destination.

## Current Status

🚧 **In Development** - This project currently has implementations in both Qt (C++) and WPF (C#/.NET). Active development is ongoing.

### ✅ Completed
- **Qt UI Framework**: Fully functional modular UI with 5 main tabs
  - Backup Sources tab for managing network/cloud paths
  - Schedule tab for configuring automated backups
  - Backup Tasks tab with progress tracking and todo list
  - Destination tab for backup locations and retention policies
  - Settings tab for compression, encryption, and notifications
- **Modular Architecture**: UI split into separate, maintainable widget files
- **Cross-platform Build**: Successfully builds on Windows with MSVC 2022

### 🔄 In Progress
- Backend implementation for UI controls
- File monitoring and change detection system

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
- **Build System**: CMake
- **Compiler**: MSVC 2022 (Windows), GCC/Clang (Linux/macOS)

### WPF Implementation (C#)
- **Framework**: .NET Framework / .NET Core
- **UI**: WPF (Windows Presentation Foundation)
- **Target Platform**: Windows

## Project Structure

```
Automate_Backup_File/
├── README.md
├── QT/                          # Qt/C++ implementation
│   └── AutomatedBackupFile/
│       ├── main.cpp             # Application entry point
│       ├── mainwindow.cpp/h/ui  # Main window (tab container)
│       ├── sourcestab.cpp/h/ui  # Backup sources management
│       ├── scheduletab.cpp/h/ui # Schedule configuration
│       ├── taskstab.cpp/h/ui    # Backup operations & tasks
│       ├── destinationtab.cpp/h/ui  # Destination settings
│       ├── settingstab.cpp/h/ui # App settings & encryption
│       ├── CMakeLists.txt       # Build configuration
│       └── build/               # Build output directory
│
└── WPF/                         # WPF/C# implementation
    └── AutomatedBackupFile/
        ├── AutomatedBackupFile.sln
        ├── MainWindow.xaml
        ├── MainWindow.xaml.cs
        └── App.xaml
```

## Installation

### Prerequisites

#### For Qt Version:
- Qt 6.9.1 or higher
- CMake 3.16 or higher
- C++17 compatible compiler
- Windows: Visual Studio 2022 or higher
- Linux: GCC 9+ or Clang 10+
- macOS: Xcode 12+

#### For WPF Version:
- Windows 10/11
- .NET Framework 4.7.2+ or .NET 6+
- Visual Studio 2019 or higher (recommended)

### Building from Source

#### Qt Version (Windows with MSVC):
```powershell
cd QT/AutomatedBackupFile
mkdir build
cd build
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.9.1\msvc2022_64"
cmake ..
cmake --build . --config Debug
```

#### Qt Version (Linux/macOS):
```bash
cd QT/AutomatedBackupFile
mkdir build && cd build
cmake ..
cmake --build .
```

#### WPF Version:
```bash
cd WPF/AutomatedBackupFile
# Open AutomatedBackupFile.sln in Visual Studio and build
# Or use command line:
msbuild AutomatedBackupFile.sln /p:Configuration=Release
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

1. **Backup Sources**: Add and manage network paths and cloud storage locations
2. **Schedule**: Configure automated backup schedules (daily, weekly, monthly, custom)
3. **Backup Tasks**: Monitor active backups, view pending tasks, and check logs
4. **Destination**: Set backup destinations and configure retention policies
5. **Settings**: Configure compression, encryption (AES-256), and notifications

*Note: UI is fully functional, backend implementation in progress*

## Roadmap

### Phase 1: UI Development ✅
- [x] Basic Qt UI framework with modular architecture
- [x] Tab-based interface (Sources, Schedule, Tasks, Destination, Settings)
- [x] Modular widget system for maintainability
- [ ] WPF UI implementation

### Phase 2: Core Functionality (In Progress)
- [ ] File system monitoring implementation
- [ ] Change detection algorithm
- [ ] Network path connection and validation
- [ ] Basic backup operation (copy files)
- [ ] Progress tracking and reporting

### Phase 3: Advanced Features
- [ ] Cloud storage integration (Google Drive, OneDrive, Dropbox, S3)
- [ ] Compression engine (ZIP, 7Z, TAR.GZ)
- [ ] Encryption system (AES-256)
- [ ] Secure credential management
- [ ] Scheduler implementation with cron-like syntax
- [ ] Todo list auto-generation

### Phase 4: Optimization & Polish
- [ ] Multi-threading for parallel backups
- [ ] Backup restoration functionality
- [ ] Comprehensive logging system
- [ ] Configuration file import/export
- [ ] Performance optimization
- [ ] Cross-platform testing (Windows, Linux, macOS)

### Phase 5: Enterprise Features
- [ ] Email notifications
- [ ] Backup verification and integrity checks
- [ ] Incremental and differential backups
- [ ] Backup profiles and templates
- [ ] Command-line interface
- [ ] REST API for remote management

## Architecture

### Modular UI Design

The Qt implementation uses a modular architecture where each tab is a separate widget:

- **MainWindow**: Container for tab widget and menu bar
- **SourcesTab**: Manages backup source locations
- **ScheduleTab**: Handles backup scheduling
- **TasksTab**: Displays backup operations and logs
- **DestinationTab**: Configures backup destinations
- **SettingsTab**: Application settings and security

This design provides:
- Easy maintenance and updates
- Better code organization
- Reduced merge conflicts in team development
- Reusable components
- Clear separation of concerns

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

### Development Guidelines
- Follow Qt coding conventions for C++ code
- Keep UI and business logic separated
- Add comments for complex algorithms
- Update documentation when adding features
- Test on multiple platforms when possible

## Security Considerations

- All credentials will be stored encrypted
- Backup files will be encrypted using industry-standard AES-256
- Secure credential transmission to network/cloud services
- No plain-text storage of sensitive information

## License

*License to be determined*

## Contact





Repository: [https://github.com/alimajidi62/Automate_Backup_File](https://github.com/alimajidi62/Automate_Backup_File)

## Acknowledgments

*To be added*

---

**Note**: This project is under active development. Features and documentation will be updated regularly as development progresses.
