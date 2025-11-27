#include "backupschedule.h"
#include <QDebug>

BackupSchedule::BackupSchedule(const QString &name, ScheduleFrequency frequency, const QTime &time)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(name)
    , m_frequency(frequency)
    , m_time(time)
    , m_enabled(true)
    , m_dayOfMonth(1)
    , m_intervalMinutes(60)
{
    // Default to all weekdays for weekly schedule
    if (frequency == ScheduleFrequency::Weekly) {
        m_weekDays = {DayOfWeek::Monday, DayOfWeek::Tuesday, DayOfWeek::Wednesday,
                      DayOfWeek::Thursday, DayOfWeek::Friday};
    }
    
    calculateInitialNextRun();
}

BackupSchedule::BackupSchedule(const QJsonObject &json)
    : m_id(json["id"].toString())
    , m_name(json["name"].toString())
    , m_frequency(static_cast<ScheduleFrequency>(json["frequency"].toInt()))
    , m_time(QTime::fromString(json["time"].toString(), "HH:mm"))
    , m_enabled(json["enabled"].toBool())
    , m_lastRun(QDateTime::fromString(json["lastRun"].toString(), Qt::ISODate))
    , m_nextRun(QDateTime::fromString(json["nextRun"].toString(), Qt::ISODate))
    , m_dayOfMonth(json["dayOfMonth"].toInt(1))
    , m_intervalMinutes(json["intervalMinutes"].toInt(60))
{
    // Load week days
    QJsonArray daysArray = json["weekDays"].toArray();
    for (const QJsonValue &val : daysArray) {
        m_weekDays.append(static_cast<DayOfWeek>(val.toInt()));
    }
    
    // Recalculate if next run is in the past
    if (m_nextRun < QDateTime::currentDateTime()) {
        calculateInitialNextRun();
    }
}

void BackupSchedule::calculateInitialNextRun()
{
    m_nextRun = calculateNextRun();
}

QDateTime BackupSchedule::calculateNextRun() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextRun = now;
    
    switch (m_frequency) {
        case ScheduleFrequency::Daily: {
            // Set to today at specified time
            nextRun.setTime(m_time);
            
            // If time has passed today, move to tomorrow
            if (nextRun <= now) {
                nextRun = nextRun.addDays(1);
            }
            break;
        }
        
        case ScheduleFrequency::Weekly: {
            // Find next matching weekday
            nextRun.setTime(m_time);
            
            for (int i = 0; i < 7; ++i) {
                QDateTime candidate = now.addDays(i);
                candidate.setTime(m_time);
                
                DayOfWeek dayOfWeek = static_cast<DayOfWeek>(candidate.date().dayOfWeek());
                
                if (m_weekDays.contains(dayOfWeek) && candidate > now) {
                    nextRun = candidate;
                    break;
                }
            }
            break;
        }
        
        case ScheduleFrequency::Monthly: {
            // Set to specified day of current month
            QDate targetDate(now.date().year(), now.date().month(), m_dayOfMonth);
            
            // If day doesn't exist in this month, use last day of month
            if (!targetDate.isValid()) {
                targetDate = QDate(now.date().year(), now.date().month(), 1).addMonths(1).addDays(-1);
            }
            
            nextRun = QDateTime(targetDate, m_time);
            
            // If time has passed this month, move to next month
            if (nextRun <= now) {
                targetDate = QDate(now.date().year(), now.date().month(), m_dayOfMonth).addMonths(1);
                if (!targetDate.isValid()) {
                    targetDate = QDate(now.date().year(), now.date().month(), 1).addMonths(2).addDays(-1);
                }
                nextRun = QDateTime(targetDate, m_time);
            }
            break;
        }
        
        case ScheduleFrequency::Custom: {
            // Add interval from last run, or from now if no last run
            if (m_lastRun.isValid()) {
                nextRun = m_lastRun.addSecs(m_intervalMinutes * 60);
            } else {
                nextRun = now.addSecs(m_intervalMinutes * 60);
            }
            break;
        }
    }
    
    return nextRun;
}

bool BackupSchedule::shouldRunNow() const
{
    if (!m_enabled) {
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    
    // Allow 1 minute tolerance
    return m_nextRun.isValid() && 
           m_nextRun <= now && 
           m_nextRun.addSecs(60) >= now;
}

QJsonObject BackupSchedule::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["frequency"] = static_cast<int>(m_frequency);
    json["time"] = m_time.toString("HH:mm");
    json["enabled"] = m_enabled;
    json["lastRun"] = m_lastRun.toString(Qt::ISODate);
    json["nextRun"] = m_nextRun.toString(Qt::ISODate);
    json["dayOfMonth"] = m_dayOfMonth;
    json["intervalMinutes"] = m_intervalMinutes;
    
    QJsonArray daysArray;
    for (DayOfWeek day : m_weekDays) {
        daysArray.append(static_cast<int>(day));
    }
    json["weekDays"] = daysArray;
    
    return json;
}

QString BackupSchedule::getFrequencyString() const
{
    switch (m_frequency) {
        case ScheduleFrequency::Daily:
            return "Daily";
        case ScheduleFrequency::Weekly:
            return "Weekly";
        case ScheduleFrequency::Monthly:
            return "Monthly";
        case ScheduleFrequency::Custom:
            return QString("Every %1 minutes").arg(m_intervalMinutes);
        default:
            return "Unknown";
    }
}
