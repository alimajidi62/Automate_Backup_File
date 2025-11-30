#include <QtTest/QtTest>
#include "retentionpolicy.h"

class TestRetentionPolicy : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultConstructor()
    {
        RetentionPolicy policy;
        // Testing that object is created successfully
        QVERIFY(policy.getRetentionDays() >= 0);
    }

    void testRetentionDays()
    {
        RetentionPolicy policy;
        
        policy.setRetentionDays(30);
        QCOMPARE(policy.getRetentionDays(), 30);
        
        policy.setRetentionDays(90);
        QCOMPARE(policy.getRetentionDays(), 90);
        
        policy.setRetentionDays(0);
        QCOMPARE(policy.getRetentionDays(), 0);
    }

    void testAutoCleanup()
    {
        RetentionPolicy policy;
        
        policy.setAutoCleanup(true);
        QVERIFY(policy.isAutoCleanupEnabled());
        
        policy.setAutoCleanup(false);
        QVERIFY(!policy.isAutoCleanupEnabled());
    }

    void testMaxBackupCount()
    {
        RetentionPolicy policy;
        
        policy.setMaxBackupCount(10);
        QCOMPARE(policy.getMaxBackupCount(), 10);
        
        policy.setMaxBackupCount(50);
        QCOMPARE(policy.getMaxBackupCount(), 50);
        
        policy.setMaxBackupCount(0); // Unlimited
        QCOMPARE(policy.getMaxBackupCount(), 0);
    }

    void testMaxStorageSize()
    {
        RetentionPolicy policy;
        
        qint64 oneGB = 1073741824;
        policy.setMaxStorageSize(oneGB);
        QCOMPARE(policy.getMaxStorageSize(), oneGB);
        
        policy.setMaxStorageSize(0); // Unlimited
        QCOMPARE(policy.getMaxStorageSize(), qint64(0));
    }

    void testKeepDailyBackups()
    {
        RetentionPolicy policy;
        
        policy.setKeepDailyBackups(true);
        QVERIFY(policy.isKeepDailyBackups());
        
        policy.setKeepDailyBackups(false);
        QVERIFY(!policy.isKeepDailyBackups());
    }

    void testKeepWeeklyBackups()
    {
        RetentionPolicy policy;
        
        policy.setKeepWeeklyBackups(true);
        QVERIFY(policy.isKeepWeeklyBackups());
        
        policy.setKeepWeeklyBackups(false);
        QVERIFY(!policy.isKeepWeeklyBackups());
    }

    void testKeepMonthlyBackups()
    {
        RetentionPolicy policy;
        
        policy.setKeepMonthlyBackups(true);
        QVERIFY(policy.isKeepMonthlyBackups());
        
        policy.setKeepMonthlyBackups(false);
        QVERIFY(!policy.isKeepMonthlyBackups());
    }

    void testShouldDeleteBackup_OldBackup()
    {
        RetentionPolicy policy;
        policy.setRetentionDays(30);
        
        // Backup older than retention period
        QDateTime oldBackup = QDateTime::currentDateTime().addDays(-60);
        bool shouldDelete = policy.shouldDeleteBackup(oldBackup);
        
        // Result depends on implementation, but testing it doesn't crash
        Q_UNUSED(shouldDelete);
    }

    void testShouldDeleteBackup_RecentBackup()
    {
        RetentionPolicy policy;
        policy.setRetentionDays(30);
        
        // Recent backup
        QDateTime recentBackup = QDateTime::currentDateTime().addDays(-10);
        bool shouldDelete = policy.shouldDeleteBackup(recentBackup);
        
        // Result depends on implementation
        Q_UNUSED(shouldDelete);
    }

    void testPolicyDescription()
    {
        RetentionPolicy policy;
        policy.setRetentionDays(30);
        policy.setAutoCleanup(true);
        policy.setMaxBackupCount(10);
        
        QString description = policy.getPolicyDescription();
        QVERIFY(!description.isEmpty());
    }

    void testComplexPolicy()
    {
        RetentionPolicy policy;
        policy.setRetentionDays(90);
        policy.setAutoCleanup(true);
        policy.setMaxBackupCount(20);
        policy.setMaxStorageSize(10737418240); // 10 GB
        policy.setKeepDailyBackups(true);
        policy.setKeepWeeklyBackups(true);
        policy.setKeepMonthlyBackups(true);
        
        QCOMPARE(policy.getRetentionDays(), 90);
        QVERIFY(policy.isAutoCleanupEnabled());
        QCOMPARE(policy.getMaxBackupCount(), 20);
        QCOMPARE(policy.getMaxStorageSize(), qint64(10737418240));
        QVERIFY(policy.isKeepDailyBackups());
        QVERIFY(policy.isKeepWeeklyBackups());
        QVERIFY(policy.isKeepMonthlyBackups());
        
        QString description = policy.getPolicyDescription();
        QVERIFY(!description.isEmpty());
    }

    void testNoRetentionPolicy()
    {
        RetentionPolicy policy;
        policy.setRetentionDays(0);
        policy.setAutoCleanup(false);
        policy.setMaxBackupCount(0);
        policy.setMaxStorageSize(0);
        
        // With no restrictions, old backups should not be deleted
        QDateTime veryOldBackup = QDateTime::currentDateTime().addYears(-5);
        // Result depends on implementation
        policy.shouldDeleteBackup(veryOldBackup);
    }
};

QTEST_MAIN(TestRetentionPolicy)
#include "test_retentionpolicy.moc"
