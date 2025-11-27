#include "schedulemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ScheduleManager::ScheduleManager(QObject *parent)
    : QObject(parent)
    , m_checkTimer(new QTimer(this))
    , m_schedulerEnabled(false)
    , m_checkIntervalSeconds(60) // Check every 60 seconds by default
{
    connect(m_checkTimer, &QTimer::timeout, this, &ScheduleManager::onCheckTimer);
}

ScheduleManager::~ScheduleManager()
{
    qDeleteAll(m_schedules);
    m_schedules.clear();
}

bool ScheduleManager::addSchedule(BackupSchedule *schedule)
{
    if (!schedule || !validateSchedule(schedule)) {
        emit error("Invalid schedule");
        return false;
    }
    
    // Check for duplicates by name
    for (const auto *existing : m_schedules) {
        if (existing->getName() == schedule->getName()) {
            emit error("Schedule with this name already exists");
            return false;
        }
    }
    
    m_schedules.append(schedule);
    emit scheduleAdded(schedule->getId());
    
    return true;
}

bool ScheduleManager::removeSchedule(const QString &scheduleId)
{
    for (int i = 0; i < m_schedules.size(); ++i) {
        if (m_schedules[i]->getId() == scheduleId) {
            BackupSchedule *schedule = m_schedules.takeAt(i);
            emit scheduleRemoved(scheduleId);
            delete schedule;
            return true;
        }
    }
    return false;
}

BackupSchedule* ScheduleManager::getSchedule(const QString &scheduleId) const
{
    for (auto *schedule : m_schedules) {
        if (schedule->getId() == scheduleId) {
            return schedule;
        }
    }
    return nullptr;
}

void ScheduleManager::setSchedulerEnabled(bool enabled)
{
    if (m_schedulerEnabled == enabled) {
        return;
    }
    
    m_schedulerEnabled = enabled;
    
    if (enabled) {
        m_checkTimer->start(m_checkIntervalSeconds * 1000);
        qDebug() << "Scheduler enabled, checking every" << m_checkIntervalSeconds << "seconds";
    } else {
        m_checkTimer->stop();
        qDebug() << "Scheduler disabled";
    }
    
    emit schedulerStateChanged(enabled);
}

void ScheduleManager::checkSchedules()
{
    if (!m_schedulerEnabled) {
        return;
    }
    
    for (BackupSchedule *schedule : m_schedules) {
        if (schedule->shouldRunNow()) {
            qDebug() << "Schedule triggered:" << schedule->getName();
            emit scheduleTriggered(schedule->getId(), schedule->getName());
            
            // Mark as run and calculate next run time
            schedule->setLastRun(QDateTime::currentDateTime());
            schedule->setNextRun(schedule->calculateNextRun());
            emit scheduleUpdated(schedule->getId());
        }
    }
}

void ScheduleManager::markScheduleRun(const QString &scheduleId)
{
    BackupSchedule *schedule = getSchedule(scheduleId);
    if (schedule) {
        schedule->setLastRun(QDateTime::currentDateTime());
        schedule->setNextRun(schedule->calculateNextRun());
        emit scheduleUpdated(scheduleId);
        qDebug() << "Schedule" << schedule->getName() << "marked as run. Next run:" << schedule->getNextRun();
    }
}

bool ScheduleManager::saveToFile(const QString &filePath)
{
    QJsonArray schedulesArray;
    
    for (const auto *schedule : m_schedules) {
        schedulesArray.append(schedule->toJson());
    }
    
    QJsonObject root;
    root["schedules"] = schedulesArray;
    root["schedulerEnabled"] = m_schedulerEnabled;
    root["checkIntervalSeconds"] = m_checkIntervalSeconds;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Failed to save schedules: " + file.errorString());
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "Saved" << m_schedules.count() << "schedules to" << filePath;
    return true;
}

bool ScheduleManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No saved schedules found (this is normal for first run)";
        return false;  // File might not exist yet, not an error
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit error("Invalid schedules file format");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Clear existing schedules
    qDeleteAll(m_schedules);
    m_schedules.clear();
    
    // Load schedules
    QJsonArray schedulesArray = root["schedules"].toArray();
    for (const QJsonValue &value : schedulesArray) {
        QJsonObject obj = value.toObject();
        auto *schedule = new BackupSchedule(obj);
        m_schedules.append(schedule);
    }
    
    // Load settings
    m_schedulerEnabled = root["schedulerEnabled"].toBool();
    m_checkIntervalSeconds = root["checkIntervalSeconds"].toInt(60);
    
    qDebug() << "Loaded" << m_schedules.count() << "schedules from" << filePath;
    
    // Start timer if scheduler was enabled
    if (m_schedulerEnabled) {
        m_checkTimer->start(m_checkIntervalSeconds * 1000);
    }
    
    return true;
}

void ScheduleManager::onCheckTimer()
{
    checkSchedules();
}

bool ScheduleManager::validateSchedule(BackupSchedule *schedule) const
{
    if (!schedule) {
        return false;
    }
    
    if (schedule->getName().isEmpty()) {
        return false;
    }
    
    if (!schedule->getTime().isValid()) {
        return false;
    }
    
    // Weekly schedule must have at least one day selected
    if (schedule->getFrequency() == ScheduleFrequency::Weekly && schedule->getWeekDays().isEmpty()) {
        return false;
    }
    
    // Monthly schedule must have valid day (1-31)
    if (schedule->getFrequency() == ScheduleFrequency::Monthly) {
        int day = schedule->getDayOfMonth();
        if (day < 1 || day > 31) {
            return false;
        }
    }
    
    // Custom schedule must have valid interval
    if (schedule->getFrequency() == ScheduleFrequency::Custom && schedule->getIntervalMinutes() < 1) {
        return false;
    }
    
    return true;
}
