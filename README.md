# Automated Backup File

A cross-platform automated backup solution that monitors network and cloud locations, detects file changes, and creates encrypted backups to local or cloud destinations.

## Overview
https://github.com/user-attachments/assets/254fbc8a-ce33-4848-b72d-c370edb6bf85

This project provides an intelligent backup system that automatically tracks file changes across network shares and cloud storage, compresses and encrypts the data, and stores it securely in your chosen destination.

## Current Status

🚧 **In Development** - This project currently has implementations in both Qt (C++) and WPF (C#/.NET). Active development is ongoing.

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
├── QT/                          # Qt/C++ implementation
│   └── AutomatedBackupFile/
│       ├── main.cpp
│       ├── mainwindow.cpp
│       ├── mainwindow.h
│       ├── mainwindow.ui
│       └── CMakeLists.txt
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

#### Qt Version:
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

*Documentation will be added as features are implemented*

## Roadmap

- [ ] Basic UI framework (Qt and WPF)
- [ ] File system monitoring implementation
- [ ] Change detection algorithm
- [ ] Network path support
- [ ] Cloud storage integration
- [ ] Compression engine
- [ ] Encryption system (AES-256)
- [ ] Credential management
- [ ] Scheduler implementation
- [ ] Todo list generator
- [ ] Multi-threading for parallel backups
- [ ] Progress tracking and reporting
- [ ] Backup restoration functionality
- [ ] Logging system
- [ ] Configuration file support
- [ ] Cross-platform testing and optimization

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

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
