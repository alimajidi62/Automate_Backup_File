#include <QtTest/QtTest>

// This is the main test runner that executes all test classes
// Each test class has its own QTEST_MAIN, so we run them individually
// This file serves as documentation for the test suite

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    
    // This file is intentionally minimal
    // Each test file has its own main() function via QTEST_MAIN macro
    // To run all tests, build the project and execute the test executable
    
    qInfo() << "AutomatedBackupFile Test Suite";
    qInfo() << "================================";
    qInfo() << "This test suite includes tests for:";
    qInfo() << "- BackupSource (test_backupsource.cpp)";
    qInfo() << "- BackupDestination (test_backupdestination.cpp)";
    qInfo() << "- BackupSchedule (test_backupschedule.cpp)";
    qInfo() << "- RetentionPolicy (test_retentionpolicy.cpp)";
    qInfo() << "- FileEncryptor (test_fileencryptor.cpp)";
    qInfo() << "- FileDecryptor (test_filedecryptor.cpp)";
    qInfo() << "- BackupEngine (test_backupengine.cpp)";
    qInfo() << "";
    qInfo() << "Each test file contains its own QTEST_MAIN macro.";
    qInfo() << "Build and run the test executable to execute all tests.";
    
    return 0;
}
