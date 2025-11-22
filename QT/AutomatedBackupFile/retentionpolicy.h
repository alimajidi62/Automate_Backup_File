#ifndef RETENTIONPOLICY_H
#define RETENTIONPOLICY_H

#include <QDateTime>

class RetentionPolicy
{
public:
    RetentionPolicy();
    
    // Getters
    int getRetentionDays() const { return m_retentionDays; }
    bool isAutoCleanupEnabled() const { return m_autoCleanup; }
    int getMaxBackupCount() const { return m_maxBackupCount; }
    qint64 getMaxStorageSize() const { return m_maxStorageSize; }
    bool isKeepDailyBackups() const { return m_keepDailyBackups; }
    bool isKeepWeeklyBackups() const { return m_keepWeeklyBackups; }
    bool isKeepMonthlyBackups() const { return m_keepMonthlyBackups; }
    
    // Setters
    void setRetentionDays(int days) { m_retentionDays = days; }
    void setAutoCleanup(bool enabled) { m_autoCleanup = enabled; }
    void setMaxBackupCount(int count) { m_maxBackupCount = count; }
    void setMaxStorageSize(qint64 size) { m_maxStorageSize = size; }
    void setKeepDailyBackups(bool keep) { m_keepDailyBackups = keep; }
    void setKeepWeeklyBackups(bool keep) { m_keepWeeklyBackups = keep; }
    void setKeepMonthlyBackups(bool keep) { m_keepMonthlyBackups = keep; }
    
    // Utility methods
    bool shouldDeleteBackup(const QDateTime &backupDate) const;
    QString getPolicyDescription() const;
    
private:
    int m_retentionDays;
    bool m_autoCleanup;
    int m_maxBackupCount;
    qint64 m_maxStorageSize;  // in bytes, 0 = unlimited
    bool m_keepDailyBackups;
    bool m_keepWeeklyBackups;
    bool m_keepMonthlyBackups;
};

#endif // RETENTIONPOLICY_H
