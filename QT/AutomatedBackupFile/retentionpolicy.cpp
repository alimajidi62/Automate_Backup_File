#include "retentionpolicy.h"

RetentionPolicy::RetentionPolicy()
    : m_retentionDays(30)
    , m_autoCleanup(false)
    , m_maxBackupCount(0)
    , m_maxStorageSize(0)
    , m_keepDailyBackups(true)
    , m_keepWeeklyBackups(true)
    , m_keepMonthlyBackups(true)
{
}

bool RetentionPolicy::shouldDeleteBackup(const QDateTime &backupDate) const
{
    if (!m_autoCleanup) {
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 daysSinceBackup = backupDate.daysTo(now);
    
    // Check retention days
    if (m_retentionDays > 0 && daysSinceBackup > m_retentionDays) {
        // Check if this backup should be kept for other reasons
        if (m_keepMonthlyBackups && backupDate.date().day() == 1) {
            return false;  // Keep monthly backups
        }
        if (m_keepWeeklyBackups && backupDate.date().dayOfWeek() == 1) {
            return false;  // Keep weekly backups (Monday)
        }
        if (m_keepDailyBackups && daysSinceBackup <= 7) {
            return false;  // Keep daily backups for last week
        }
        return true;
    }
    
    return false;
}

QString RetentionPolicy::getPolicyDescription() const
{
    QStringList parts;
    
    if (m_retentionDays > 0) {
        parts << QString("Keep for %1 days").arg(m_retentionDays);
    }
    
    if (m_maxBackupCount > 0) {
        parts << QString("Max %1 backups").arg(m_maxBackupCount);
    }
    
    if (m_autoCleanup) {
        parts << "Auto-cleanup enabled";
    }
    
    if (parts.isEmpty()) {
        return "No retention policy";
    }
    
    return parts.join(", ");
}
