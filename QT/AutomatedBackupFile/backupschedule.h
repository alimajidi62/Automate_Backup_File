#ifndef BACKUPSCHEDULE_H
#define BACKUPSCHEDULE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

enum class ScheduleFrequency {
    Daily,
    Weekly,
    Monthly,
    Custom
};

enum class DayOfWeek {
    Monday = 1,
    Tuesday = 2,
    Wednesday = 3,
    Thursday = 4,
    Friday = 5,
    Saturday = 6,
    Sunday = 7
};

class BackupSchedule
{
public:
    BackupSchedule(const QString &name, ScheduleFrequency frequency, const QTime &time);
    BackupSchedule(const QJsonObject &json);
    
    // Getters
    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    ScheduleFrequency getFrequency() const { return m_frequency; }
    QTime getTime() const { return m_time; }
    bool isEnabled() const { return m_enabled; }
    QDateTime getLastRun() const { return m_lastRun; }
    QDateTime getNextRun() const { return m_nextRun; }
    QList<DayOfWeek> getWeekDays() const { return m_weekDays; }
    int getDayOfMonth() const { return m_dayOfMonth; }
    int getIntervalMinutes() const { return m_intervalMinutes; }
    
    // Setters
    void setName(const QString &name) { m_name = name; }
    void setFrequency(ScheduleFrequency frequency) { m_frequency = frequency; }
    void setTime(const QTime &time) { m_time = time; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    void setLastRun(const QDateTime &lastRun) { m_lastRun = lastRun; }
    void setNextRun(const QDateTime &nextRun) { m_nextRun = nextRun; }
    void setWeekDays(const QList<DayOfWeek> &days) { m_weekDays = days; }
    void setDayOfMonth(int day) { m_dayOfMonth = day; }
    void setIntervalMinutes(int minutes) { m_intervalMinutes = minutes; }
    
    // Calculate next run time based on current settings
    QDateTime calculateNextRun() const;
    
    // Check if schedule should run now
    bool shouldRunNow() const;
    
    // Serialization
    QJsonObject toJson() const;
    
    // Get frequency as string
    QString getFrequencyString() const;

private:
    QString m_id;
    QString m_name;
    ScheduleFrequency m_frequency;
    QTime m_time;
    bool m_enabled;
    QDateTime m_lastRun;
    QDateTime m_nextRun;
    
    // For weekly schedules
    QList<DayOfWeek> m_weekDays;
    
    // For monthly schedules
    int m_dayOfMonth; // 1-31
    
    // For custom interval schedules
    int m_intervalMinutes; // Custom interval in minutes
    
    void calculateInitialNextRun();
};

#endif // BACKUPSCHEDULE_H
