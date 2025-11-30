#include <QtTest/QtTest>
#include "backupdestination.h"

class TestBackupDestination : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstructor()
    {
        BackupDestination dest;
        QVERIFY(!dest.getId().isEmpty());
        QVERIFY(dest.getPath().isEmpty());
    }

    void testParameterizedConstructor()
    {
        BackupDestination dest("D:/backup", DestinationType::Local);
        QCOMPARE(dest.getPath(), QString("D:/backup"));
        QCOMPARE(dest.getType(), DestinationType::Local);
        QVERIFY(!dest.getId().isEmpty());
    }

    void testSettersAndGetters()
    {
        BackupDestination dest;
        
        dest.setPath("E:/backups");
        QCOMPARE(dest.getPath(), QString("E:/backups"));
        
        dest.setType(DestinationType::Network);
        QCOMPARE(dest.getType(), DestinationType::Network);
        
        dest.setStatus(DestinationStatus::Available);
        QCOMPARE(dest.getStatus(), DestinationStatus::Available);
        
        dest.setFreeSpace(1073741824); // 1 GB
        QCOMPARE(dest.getFreeSpace(), qint64(1073741824));
        
        dest.setTotalSpace(10737418240); // 10 GB
        QCOMPARE(dest.getTotalSpace(), qint64(10737418240));
        
        dest.setUsername("admin");
        QCOMPARE(dest.getUsername(), QString("admin"));
        
        dest.setPassword("secure123");
        QCOMPARE(dest.getPassword(), QString("secure123"));
        
        dest.setEnabled(true);
        QVERIFY(dest.isEnabled());
        
        dest.setEnabled(false);
        QVERIFY(!dest.isEnabled());
    }

    void testTypeString()
    {
        BackupDestination localDest("C:/backup", DestinationType::Local);
        QVERIFY(!localDest.getTypeString().isEmpty());
        
        BackupDestination networkDest("//server/backup", DestinationType::Network);
        QVERIFY(!networkDest.getTypeString().isEmpty());
        
        BackupDestination cloudDest("cloud://backup", DestinationType::Cloud);
        QVERIFY(!cloudDest.getTypeString().isEmpty());
    }

    void testStatusString()
    {
        BackupDestination dest;
        
        dest.setStatus(DestinationStatus::Available);
        QVERIFY(!dest.getStatusString().isEmpty());
        
        dest.setStatus(DestinationStatus::Unavailable);
        QVERIFY(!dest.getStatusString().isEmpty());
        
        dest.setStatus(DestinationStatus::Checking);
        QVERIFY(!dest.getStatusString().isEmpty());
        
        dest.setStatus(DestinationStatus::Error);
        QVERIFY(!dest.getStatusString().isEmpty());
    }

    void testFreeSpaceString()
    {
        BackupDestination dest;
        dest.setFreeSpace(1024); // 1 KB
        QString spaceStr = dest.getFreeSpaceString();
        QVERIFY(!spaceStr.isEmpty());
    }

    void testFreeSpacePercentage()
    {
        BackupDestination dest;
        dest.setTotalSpace(100);
        dest.setFreeSpace(50);
        
        double percentage = dest.getFreeSpacePercentage();
        QCOMPARE(percentage, 50.0);
    }

    void testFreeSpacePercentageZeroTotal()
    {
        BackupDestination dest;
        dest.setTotalSpace(0);
        dest.setFreeSpace(0);
        
        // Should handle division by zero gracefully
        double percentage = dest.getFreeSpacePercentage();
        QVERIFY(percentage >= 0.0);
    }

    void testIsValid()
    {
        BackupDestination validDest("C:/backup", DestinationType::Local);
        QVERIFY(validDest.isValid());
        
        BackupDestination invalidDest("", DestinationType::Local);
        // Testing that it doesn't crash
        invalidDest.isValid();
    }

    void testLastCheckedDateTime()
    {
        BackupDestination dest;
        QDateTime now = QDateTime::currentDateTime();
        dest.setLastChecked(now);
        
        QCOMPARE(dest.getLastChecked(), now);
    }

    void testNetworkDestinationWithCredentials()
    {
        BackupDestination dest("//192.168.1.100/backup", DestinationType::Network);
        dest.setUsername("networkuser");
        dest.setPassword("networkpass");
        
        QCOMPARE(dest.getUsername(), QString("networkuser"));
        QCOMPARE(dest.getPassword(), QString("networkpass"));
        QCOMPARE(dest.getType(), DestinationType::Network);
    }

    void testMultipleDestinations()
    {
        BackupDestination dest1("C:/backup1", DestinationType::Local);
        BackupDestination dest2("D:/backup2", DestinationType::Local);
        
        // IDs should be unique
        QVERIFY(dest1.getId() != dest2.getId());
    }
};

QTEST_MAIN(TestBackupDestination)
#include "test_backupdestination.moc"
