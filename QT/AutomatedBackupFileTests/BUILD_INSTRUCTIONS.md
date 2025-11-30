# Build Instructions for AutomatedBackupFileTests

## Option 1: Build with Qt Creator (Recommended)

1. Open Qt Creator
2. File → Open File or Project
3. Navigate to: `c:\testcode\Automate_Backup_File\QT\AutomatedBackupFileTests\CMakeLists.txt`
4. Select your Qt Kit (Desktop Qt 6.9.1 MinGW 64-bit)
5. Click "Configure Project"
6. Build → Build Project "AutomatedBackupFileTests"
7. Run tests from the Test Results pane in Qt Creator

## Option 2: Build from Command Line

### Prerequisites
Add Qt and MinGW to your PATH:

```powershell
$env:Path = "C:\Qt\6.9.1\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:Path"
```

### Build Steps

```powershell
cd c:\testcode\Automate_Backup_File\QT\AutomatedBackupFileTests
mkdir build_mingw -ErrorAction SilentlyContinue
cd build_mingw
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64
mingw32-make
```

### Run All Tests

```powershell
ctest --verbose
```

### Run Individual Tests

```powershell
.\test_backupsource.exe
.\test_backupdestination.exe
.\test_backupschedule.exe
.\test_retentionpolicy.exe
.\test_fileencryptor.exe
.\test_filedecryptor.exe
.\test_backupengine.exe
```

## Troubleshooting

If you get compiler errors, make sure:
1. MinGW is in your PATH
2. Qt is properly installed
3. You're using the matching Qt version and compiler

To verify your PATH:
```powershell
where.exe g++
where.exe qmake
```
