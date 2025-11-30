# AutomatedBackupFile Test Suite

This is a comprehensive test suite for the AutomatedBackupFile Qt application using the Qt Test framework.

## Overview

The test suite provides unit tests for all core classes in the AutomatedBackupFile project, ensuring reliability and correctness of the backup functionality.

## Test Coverage

### Tested Classes

1. **BackupSource** (`test_backupsource.cpp`)
   - Constructor and initialization
   - Getters and setters for all properties
   - Type and status string conversions
   - JSON serialization and deserialization
   - Path validation

2. **BackupDestination** (`test_backupdestination.cpp`)
   - Constructor variants
   - Property management (path, type, status, space)
   - Free space calculations and percentage
   - Network destination credentials
   - Validation methods

3. **BackupSchedule** (`test_backupschedule.cpp`)
   - Schedule creation (Daily, Weekly, Monthly, Custom)
   - Time-based scheduling
   - Next run calculations
   - JSON serialization
   - Schedule validation

4. **RetentionPolicy** (`test_retentionpolicy.cpp`)
   - Retention period settings
   - Backup count limits
   - Storage size limits
   - Backup type retention (daily/weekly/monthly)
   - Deletion criteria evaluation

5. **FileEncryptor** (`test_fileencryptor.cpp`)
   - Password loading and setting
   - Single file encryption
   - Directory encryption (recursive)
   - Empty file handling
   - Large file encryption

6. **FileDecryptor** (`test_filedecryptor.cpp`)
   - Password management
   - File decryption
   - Directory decryption
   - Encrypt/decrypt cycle verification
   - Wrong password handling

7. **BackupEngine** (`test_backupengine.cpp`)
   - Backup engine initialization
   - Backup process execution
   - Progress tracking
   - Multiple source-destination pairs
   - Subdirectory handling
   - Stop/cancel operations
   - Signal emission

## Building the Tests

### Prerequisites

- Qt 5 or Qt 6 (with Qt Test module)
- CMake 3.16 or higher
- C++17 compatible compiler
- Main AutomatedBackupFile project sources

### Build Instructions

1. Open a terminal/PowerShell in the test directory:
   ```powershell
   cd c:\testcode\Automate_Backup_File\QT\AutomatedBackupFileTests
   ```

2. Create a build directory:
   ```powershell
   mkdir build; cd build
   ```

3. Configure with CMake:
   ```powershell
   cmake ..
   ```

4. Build the tests:
   ```powershell
   cmake --build .
   ```

## Running the Tests

### Run All Tests

After building, use CTest to run all tests:

```powershell
ctest --verbose
```

Or run all tests with output on failure:

```powershell
ctest --output-on-failure
```

### Run Individual Tests

Each test class is built as a separate executable:

```powershell
.\bin\test_backupsource.exe
.\bin\test_backupdestination.exe
.\bin\test_backupschedule.exe
.\bin\test_retentionpolicy.exe
.\bin\test_fileencryptor.exe
.\bin\test_filedecryptor.exe
.\bin\test_backupengine.exe
```

### Run Tests in Qt Creator

1. Open `CMakeLists.txt` in Qt Creator
2. Build the project
3. Click on "Tests" in the left sidebar
4. Run individual tests or all tests

### Command Line Options

Qt Test provides various command line options for individual test executables:

```powershell
# Run with detailed output
.\bin\test_backupsource.exe -v2

# Run specific test function
.\bin\test_backupsource.exe testConstructor

# Generate XML output
.\bin\test_backupsource.exe -o results.xml -xunitxml
```

## Test Structure

Each test file follows this pattern:

```cpp
class TestClassName : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();      // Run once before all tests
    void cleanupTestCase();   // Run once after all tests
    void init();              // Run before each test (optional)
    void cleanup();           // Run after each test (optional)
    void testSomething();     // Actual test cases
};
```

## Test Features

### Temporary Directories

Tests that involve file operations use `QTemporaryDir` to create isolated test environments that are automatically cleaned up.

### Signal Testing

Tests use `QSignalSpy` to verify that signals are emitted correctly during operations.

### Assertions

Common Qt Test assertions used:
- `QVERIFY(condition)` - Verify boolean condition
- `QCOMPARE(actual, expected)` - Compare two values
- `QVERIFY2(condition, message)` - Verify with custom message
- `QTEST(data, "function")` - Data-driven testing

## Continuous Integration

This test suite can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Build Tests
  run: |
    cd QT/AutomatedBackupFileTests
    mkdir build && cd build
    cmake ..
    cmake --build .

- name: Run Tests
  run: |
    cd QT/AutomatedBackupFileTests/build
    ctest --output-on-failure
```

## Contributing

When adding new features to AutomatedBackupFile:

1. Create corresponding test cases
2. Ensure all tests pass before submitting changes
3. Aim for high code coverage
4. Test both success and failure scenarios

## Test Best Practices

- **Isolation**: Each test should be independent
- **Clear naming**: Use descriptive test function names
- **Cleanup**: Use `cleanupTestCase()` and `cleanup()` properly
- **Coverage**: Test normal, edge, and error cases
- **Speed**: Keep tests fast for quick feedback

## Debugging Tests

### In Qt Creator
1. Set breakpoints in test code
2. Right-click test in Tests view
3. Select "Debug"

### Command Line
```powershell
# Windows - Debug specific test
windbg .\bin\test_backupsource.exe

# With GDB (if available)
gdb --args .\bin\test_backupsource.exe
```

## Known Limitations

- Some tests use `QTest::qWait()` which may need adjustment on slower systems
- Network-related tests require appropriate file system permissions
- Encryption/decryption tests depend on the implementation details of the crypto system

## Future Enhancements

- [ ] Add performance benchmarks
- [ ] Add integration tests
- [ ] Mock network operations
- [ ] Add code coverage reporting
- [ ] Add stress tests for large file operations
- [ ] Add UI tests (if GUI testing framework is integrated)

## License

This test suite follows the same license as the main AutomatedBackupFile project.

## Contact

For questions or issues with the test suite, please refer to the main project documentation or repository.
