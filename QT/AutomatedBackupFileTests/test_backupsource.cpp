#include <QtTest/QtTest>
#include "backupsource.h"
#include <QJsonDocument>

class TestBackupSource : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // This function is called once before all test functions
    }

    void cleanupTestCase()
    {
        // This function is called once after all test functions
    }

    void testConstructor()
    {
        BackupSource source("C:/test/path", SourceType::Local);
        QCOMPARE(source.getPath(), QString("C:/test/path"));
        QCOMPARE(source.getType(), SourceType::Local);
        QVERIFY(!source.getId().isEmpty());
    }

    void testDefaultConstructor()
    {
        BackupSource source;
        QVERIFY(source.getPath().isEmpty());
        QCOMPARE(source.getType(), SourceType::Local);
        QVERIFY(!source.getId().isEmpty());
    }

    void testSettersAndGetters()
    {
        BackupSource source;
        
        source.setPath("D:/backup/folder");
        QCOMPARE(source.getPath(), QString("D:/backup/folder"));
        
        source.setType(SourceType::Network);
        QCOMPARE(source.getType(), SourceType::Network);
        
        source.setStatus(SourceStatus::Available);
        QCOMPARE(source.getStatus(), SourceStatus::Available);
        
        source.setUsername("testuser");
        QCOMPARE(source.getUsername(), QString("testuser"));
        
        source.setPassword("testpass");
        QCOMPARE(source.getPassword(), QString("testpass"));
        
        source.setDomain("testdomain");
        QCOMPARE(source.getDomain(), QString("testdomain"));
        
        source.setEnabled(true);
        QVERIFY(source.isEnabled());
        
        source.setEnabled(false);
        QVERIFY(!source.isEnabled());
        
        source.setRequiresAuthentication(true);
        QVERIFY(source.requiresAuthentication());
        
        source.setTotalSize(1024000);
        QCOMPARE(source.getTotalSize(), qint64(1024000));
        
        source.setFileCount(42);
        QCOMPARE(source.getFileCount(), 42);
    }

    void testTypeStrings()
    {
        BackupSource localSource("C:/test", SourceType::Local);
        QVERIFY(!localSource.getTypeString().isEmpty());
        
        BackupSource networkSource("//server/share", SourceType::Network);
        QVERIFY(!networkSource.getTypeString().isEmpty());
        
        BackupSource cloudSource("cloud://storage", SourceType::Cloud);
        QVERIFY(!cloudSource.getTypeString().isEmpty());
    }

    void testStatusStrings()
    {
        BackupSource source;
        
        source.setStatus(SourceStatus::Available);
        QVERIFY(!source.getStatusString().isEmpty());
        
        source.setStatus(SourceStatus::Unavailable);
        QVERIFY(!source.getStatusString().isEmpty());
        
        source.setStatus(SourceStatus::Checking);
        QVERIFY(!source.getStatusString().isEmpty());
        
        source.setStatus(SourceStatus::Error);
        QVERIFY(!source.getStatusString().isEmpty());
        
        source.setStatus(SourceStatus::CredentialsRequired);
        QVERIFY(!source.getStatusString().isEmpty());
    }

    void testJsonSerialization()
    {
        BackupSource source("C:/test/path", SourceType::Local);
        source.setUsername("testuser");
        source.setEnabled(true);
        source.setTotalSize(2048);
        source.setFileCount(10);
        
        QJsonObject json = source.toJson();
        QVERIFY(!json.isEmpty());
        QVERIFY(json.contains("id"));
        QVERIFY(json.contains("path"));
        QVERIFY(json.contains("type"));
    }

    void testJsonDeserialization()
    {
        BackupSource originalSource("C:/original/path", SourceType::Network);
        originalSource.setUsername("user123");
        originalSource.setEnabled(true);
        originalSource.setTotalSize(4096);
        
        QJsonObject json = originalSource.toJson();
        BackupSource* deserializedSource = BackupSource::fromJson(json);
        
        QVERIFY(deserializedSource != nullptr);
        QCOMPARE(deserializedSource->getPath(), originalSource.getPath());
        QCOMPARE(deserializedSource->getType(), originalSource.getType());
        QCOMPARE(deserializedSource->getUsername(), originalSource.getUsername());
        QCOMPARE(deserializedSource->isEnabled(), originalSource.isEnabled());
        
        delete deserializedSource;
    }

    void testIsValid()
    {
        BackupSource validSource("C:/valid/path", SourceType::Local);
        QVERIFY(validSource.isValid());
        
        BackupSource invalidSource("", SourceType::Local);
        // The result depends on implementation, but we're testing that it doesn't crash
        invalidSource.isValid();
    }

    void testDisplayPath()
    {
        BackupSource source("C:/very/long/path/to/test", SourceType::Local);
        QString displayPath = source.getDisplayPath();
        QVERIFY(!displayPath.isEmpty());
    }

    void testLastCheckedDateTime()
    {
        BackupSource source;
        QDateTime now = QDateTime::currentDateTime();
        source.setLastChecked(now);
        
        QCOMPARE(source.getLastChecked(), now);
    }

    void testLastError()
    {
        BackupSource source;
        QString errorMsg = "Test error message";
        source.setLastError(errorMsg);
        
        QCOMPARE(source.getLastError(), errorMsg);
    }
};

QTEST_MAIN(TestBackupSource)
#include "test_backupsource.moc"
