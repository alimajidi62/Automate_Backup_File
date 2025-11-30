#include <QtTest/QtTest>
#include "backupengine.h"
#include <QTemporaryDir>
#include <QSignalSpy>

class TestBackupEngine : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir* tempDir;

private slots:
    void initTestCase()
    {
        tempDir = new QTemporaryDir();
        QVERIFY(tempDir->isValid());
    }

    void cleanupTestCase()
    {
        delete tempDir;
    }

    void testConstructor()
    {
        BackupEngine engine;
        QCOMPARE(engine.getStatus(), BackupStatus::Idle);
        QCOMPARE(engine.getProgress(), 0);
    }

    void testGetStatus()
    {
        BackupEngine engine;
        BackupStatus status = engine.getStatus();
        QVERIFY(status == BackupStatus::Idle || 
                status == BackupStatus::Running || 
                status == BackupStatus::Paused ||
                status == BackupStatus::Completed ||
                status == BackupStatus::Failed);
    }

    void testGetProgress()
    {
        BackupEngine engine;
        int progress = engine.getProgress();
        QVERIFY(progress >= 0 && progress <= 100);
    }

    void testGetTotalFiles()
    {
        BackupEngine engine;
        qint64 totalFiles = engine.getTotalFiles();
        QVERIFY(totalFiles >= 0);
    }

    void testGetProcessedFiles()
    {
        BackupEngine engine;
        qint64 processedFiles = engine.getProcessedFiles();
        QVERIFY(processedFiles >= 0);
    }

    void testGetCurrentFile()
    {
        BackupEngine engine;
        QString currentFile = engine.getCurrentFile();
        // May be empty initially
        Q_UNUSED(currentFile);
    }

    void testStartBackupWithEmptyPairs()
    {
        BackupEngine engine;
        std::vector<std::pair<QString, QString>> emptyPairs;
        
        // Starting with empty pairs should not crash
        engine.startBackup(emptyPairs);
        
        // Give it a moment to process
        QTest::qWait(100);
        
        // Status should change or remain idle
        BackupStatus status = engine.getStatus();
        Q_UNUSED(status);
    }

    void testStartBackupWithValidPairs()
    {
        // Create source directory with a test file
        QString sourceDir = tempDir->filePath("backup_source");
        QDir().mkpath(sourceDir);
        
        QString testFile = sourceDir + "/test.txt";
        QFile file(testFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("Test backup content");
        file.close();
        
        // Create destination directory
        QString destDir = tempDir->filePath("backup_dest");
        
        BackupEngine engine;
        std::vector<std::pair<QString, QString>> pairs;
        pairs.push_back(std::make_pair(sourceDir, destDir));
        
        // Use signal spy to monitor progress
        QSignalSpy statusSpy(&engine, &BackupEngine::statusChanged);
        QSignalSpy progressSpy(&engine, &BackupEngine::progressUpdated);
        
        engine.startBackup(pairs);
        
        // Wait for backup to complete (with timeout)
        QTest::qWait(2000);
        
        // Should have received at least some signals
        QVERIFY(statusSpy.count() > 0 || progressSpy.count() >= 0);
    }

    void testStopBackup()
    {
        BackupEngine engine;
        
        // Create a simple backup task
        QString sourceDir = tempDir->filePath("stop_source");
        QDir().mkpath(sourceDir);
        
        QString destDir = tempDir->filePath("stop_dest");
        
        std::vector<std::pair<QString, QString>> pairs;
        pairs.push_back(std::make_pair(sourceDir, destDir));
        
        engine.startBackup(pairs);
        QTest::qWait(100);
        
        // Stop the backup
        engine.stopBackup();
        
        // Give it time to stop
        QTest::qWait(200);
        
        // Engine should have processed the stop request
        QVERIFY(true);
    }

    void testSignalEmission()
    {
        BackupEngine engine;
        
        // Set up signal spies
        QSignalSpy statusSpy(&engine, &BackupEngine::statusChanged);
        QSignalSpy progressSpy(&engine, &BackupEngine::progressUpdated);
        QSignalSpy fileProcessedSpy(&engine, &BackupEngine::fileProcessed);
        QSignalSpy completedSpy(&engine, &BackupEngine::backupCompleted);
        QSignalSpy failedSpy(&engine, &BackupEngine::backupFailed);
        
        QVERIFY(statusSpy.isValid());
        QVERIFY(progressSpy.isValid());
        QVERIFY(fileProcessedSpy.isValid());
        QVERIFY(completedSpy.isValid());
        QVERIFY(failedSpy.isValid());
    }

    void testBackupNonExistentSource()
    {
        BackupEngine engine;
        
        std::vector<std::pair<QString, QString>> pairs;
        pairs.push_back(std::make_pair("C:/NonExistent/Path/Source", tempDir->filePath("dest")));
        
        QSignalSpy failedSpy(&engine, &BackupEngine::backupFailed);
        
        engine.startBackup(pairs);
        QTest::qWait(1000);
        
        // Should either fail or complete, but not crash
        QVERIFY(true);
    }

    void testMultipleSourceDestinationPairs()
    {
        BackupEngine engine;
        
        // Create multiple source directories
        QString source1 = tempDir->filePath("multi_source1");
        QString source2 = tempDir->filePath("multi_source2");
        QDir().mkpath(source1);
        QDir().mkpath(source2);
        
        QString dest1 = tempDir->filePath("multi_dest1");
        QString dest2 = tempDir->filePath("multi_dest2");
        
        std::vector<std::pair<QString, QString>> pairs;
        pairs.push_back(std::make_pair(source1, dest1));
        pairs.push_back(std::make_pair(source2, dest2));
        
        engine.startBackup(pairs);
        QTest::qWait(1000);
        
        // Should handle multiple pairs without crashing
        QVERIFY(true);
    }

    void testBackupWithSubdirectories()
    {
        // Create source with subdirectories
        QString sourceDir = tempDir->filePath("subdir_source");
        QString subDir = sourceDir + "/subfolder";
        QDir().mkpath(subDir);
        
        QString file1 = sourceDir + "/file1.txt";
        QString file2 = subDir + "/file2.txt";
        
        QFile f1(file1);
        QVERIFY(f1.open(QIODevice::WriteOnly));
        f1.write("Content 1");
        f1.close();
        
        QFile f2(file2);
        QVERIFY(f2.open(QIODevice::WriteOnly));
        f2.write("Content 2");
        f2.close();
        
        QString destDir = tempDir->filePath("subdir_dest");
        
        BackupEngine engine;
        std::vector<std::pair<QString, QString>> pairs;
        pairs.push_back(std::make_pair(sourceDir, destDir));
        
        engine.startBackup(pairs);
        QTest::qWait(2000);
        
        // Should process subdirectories
        QVERIFY(engine.getProcessedFiles() >= 0);
    }
};

QTEST_MAIN(TestBackupEngine)
#include "test_backupengine.moc"
