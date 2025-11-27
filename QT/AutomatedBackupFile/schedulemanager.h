#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include "backupschedule.h"

class ScheduleManager : public QObject
{
    Q_OBJECT

public:
    explicit ScheduleManager(QObject *parent = nullptr);
    ~ScheduleManager();
    
    // Schedule management
    bool addSchedule(BackupSchedule *schedule);
    bool removeSchedule(const QString &scheduleId);
    BackupSchedule* getSchedule(const QString &scheduleId) const;
    QList<BackupSchedule*> getAllSchedules() const { return m_schedules; }
    int getScheduleCount() const { return m_schedules.count(); }
    
    // Scheduler control
    void setSchedulerEnabled(bool enabled);
    bool isSchedulerEnabled() const { return m_schedulerEnabled; }
    void setCheckInterval(int seconds) { m_checkIntervalSeconds = seconds; }
    int getCheckInterval() const { return m_checkIntervalSeconds; }
    
    // Manual operations
    void checkSchedules(); // Manually check if any schedule should run
    void markScheduleRun(const QString &scheduleId); // Mark schedule as run
    
    // Persistence
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

signals:
    void scheduleAdded(const QString &scheduleId);
    void scheduleRemoved(const QString &scheduleId);
    void scheduleUpdated(const QString &scheduleId);
    void scheduleTriggered(const QString &scheduleId, const QString &scheduleName);
    void schedulerStateChanged(bool enabled);
    void error(const QString &message);

private slots:
    void onCheckTimer();

private:
    QList<BackupSchedule*> m_schedules;
    QTimer *m_checkTimer;
    bool m_schedulerEnabled;
    int m_checkIntervalSeconds;
    
    bool validateSchedule(BackupSchedule *schedule) const;
};

#endif // SCHEDULEMANAGER_H
