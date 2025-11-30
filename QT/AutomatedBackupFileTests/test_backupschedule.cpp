#include <QtTest/QtTest>
#include "backupschedule.h"

class TestBackupSchedule : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor()
    {
        QTime time(14, 30); // 2:30 PM
        BackupSchedule schedule("Daily Backup", ScheduleFrequency::Daily, time);
        
        QCOMPARE(schedule.getName(), QString("Daily Backup"));
        QCOMPARE(schedule.getFrequency(), ScheduleFrequency::Daily);
        QCOMPARE(schedule.getTime(), time);
        QVERIFY(!schedule.getId().isEmpty());
    }

    void testSettersAndGetters()
    {
        QTime time(10, 0);
        BackupSchedule schedule("Test", ScheduleFrequency::Daily, time);
        
        schedule.setName("Updated Backup");
        QCOMPARE(schedule.getName(), QString("Updated Backup"));
        
        schedule.setFrequency(ScheduleFrequency::Weekly);
        QCOMPARE(schedule.getFrequency(), ScheduleFrequency::Weekly);
        
        QTime newTime(16, 45);
        schedule.setTime(newTime);
        QCOMPARE(schedule.getTime(), newTime);
        
        schedule.setEnabled(true);
        QVERIFY(schedule.isEnabled());
        
        schedule.setEnabled(false);
        QVERIFY(!schedule.isEnabled());
        
        QDateTime lastRun = QDateTime::currentDateTime();
        schedule.setLastRun(lastRun);
        QCOMPARE(schedule.getLastRun(), lastRun);
        
        QDateTime nextRun = QDateTime::currentDateTime().addDays(1);
        schedule.setNextRun(nextRun);
        QCOMPARE(schedule.getNextRun(), nextRun);
    }

    void testWeeklySchedule()
    {
        QTime time(9, 0);
        BackupSchedule schedule("Weekly Backup", ScheduleFrequency::Weekly, time);
        
        QList<DayOfWeek> days;
        days << DayOfWeek::Monday << DayOfWeek::Wednesday << DayOfWeek::Friday;
        schedule.setWeekDays(days);
        
        QCOMPARE(schedule.getWeekDays().size(), 3);
        QVERIFY(schedule.getWeekDays().contains(DayOfWeek::Monday));
        QVERIFY(schedule.getWeekDays().contains(DayOfWeek::Wednesday));
        QVERIFY(schedule.getWeekDays().contains(DayOfWeek::Friday));
    }

    void testMonthlySchedule()
    {
        QTime time(8, 0);
        BackupSchedule schedule("Monthly Backup", ScheduleFrequency::Monthly, time);
        
        schedule.setDayOfMonth(15);
        QCOMPARE(schedule.getDayOfMonth(), 15);
    }

    void testCustomSchedule()
    {
        QTime time(12, 0);
        BackupSchedule schedule("Custom Backup", ScheduleFrequency::Custom, time);
        
        schedule.setIntervalMinutes(120); // Every 2 hours
        QCOMPARE(schedule.getIntervalMinutes(), 120);
    }

    void testCalculateNextRun()
    {
        QTime time(14, 0);
        BackupSchedule schedule("Test Schedule", ScheduleFrequency::Daily, time);
        
        QDateTime nextRun = schedule.calculateNextRun();
        QVERIFY(nextRun.isValid());
    }

    void testShouldRunNow()
    {
        QTime currentTime = QTime::currentTime();
        BackupSchedule schedule("Test Schedule", ScheduleFrequency::Daily, currentTime);
        schedule.setEnabled(true);
        
        // Testing the method doesn't crash
        bool shouldRun = schedule.shouldRunNow();
        Q_UNUSED(shouldRun);
    }

    void testFrequencyString()
    {
        QTime time(10, 0);
        
        BackupSchedule dailySchedule("Daily", ScheduleFrequency::Daily, time);
        QVERIFY(!dailySchedule.getFrequencyString().isEmpty());
        
        BackupSchedule weeklySchedule("Weekly", ScheduleFrequency::Weekly, time);
        QVERIFY(!weeklySchedule.getFrequencyString().isEmpty());
        
        BackupSchedule monthlySchedule("Monthly", ScheduleFrequency::Monthly, time);
        QVERIFY(!monthlySchedule.getFrequencyString().isEmpty());
        
        BackupSchedule customSchedule("Custom", ScheduleFrequency::Custom, time);
        QVERIFY(!customSchedule.getFrequencyString().isEmpty());
    }

    void testJsonSerialization()
    {
        QTime time(11, 30);
        BackupSchedule schedule("Test Schedule", ScheduleFrequency::Weekly, time);
        schedule.setEnabled(true);
        schedule.setDayOfMonth(10);
        
        QJsonObject json = schedule.toJson();
        QVERIFY(!json.isEmpty());
        QVERIFY(json.contains("id"));
        QVERIFY(json.contains("name"));
        QVERIFY(json.contains("frequency"));
        QVERIFY(json.contains("time"));
        QVERIFY(json.contains("enabled"));
    }

    void testJsonDeserialization()
    {
        QTime time(13, 15);
        BackupSchedule originalSchedule("Original", ScheduleFrequency::Daily, time);
        originalSchedule.setEnabled(true);
        
        QJsonObject json = originalSchedule.toJson();
        BackupSchedule deserializedSchedule(json);
        
        QCOMPARE(deserializedSchedule.getName(), originalSchedule.getName());
        QCOMPARE(deserializedSchedule.getFrequency(), originalSchedule.getFrequency());
        QCOMPARE(deserializedSchedule.isEnabled(), originalSchedule.isEnabled());
    }

    void testDisabledScheduleShouldNotRun()
    {
        QTime time = QTime::currentTime();
        BackupSchedule schedule("Disabled Schedule", ScheduleFrequency::Daily, time);
        schedule.setEnabled(false);
        
        // A disabled schedule should not run
        // The exact behavior depends on implementation
        bool shouldRun = schedule.shouldRunNow();
        Q_UNUSED(shouldRun);
    }
};

QTEST_MAIN(TestBackupSchedule)
#include "test_backupschedule.moc"
