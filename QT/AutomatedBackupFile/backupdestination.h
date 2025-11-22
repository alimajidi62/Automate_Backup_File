#ifndef BACKUPDESTINATION_H
#define BACKUPDESTINATION_H

#include <QString>
#include <QDateTime>

enum class DestinationType {
    Local,
    Network,
    Cloud
};

enum class DestinationStatus {
    Available,
    Unavailable,
    Checking,
    Error
};

class BackupDestination
{
public:
    BackupDestination();
    BackupDestination(const QString &path, DestinationType type);
    
    // Getters
    QString getId() const { return m_id; }
    QString getPath() const { return m_path; }
    DestinationType getType() const { return m_type; }
    DestinationStatus getStatus() const { return m_status; }
    qint64 getFreeSpace() const { return m_freeSpace; }
    qint64 getTotalSpace() const { return m_totalSpace; }
    QDateTime getLastChecked() const { return m_lastChecked; }
    QString getUsername() const { return m_username; }
    QString getPassword() const { return m_password; }
    bool isEnabled() const { return m_enabled; }
    
    // Setters
    void setPath(const QString &path) { m_path = path; }
    void setType(DestinationType type) { m_type = type; }
    void setStatus(DestinationStatus status) { m_status = status; }
    void setFreeSpace(qint64 freeSpace) { m_freeSpace = freeSpace; }
    void setTotalSpace(qint64 totalSpace) { m_totalSpace = totalSpace; }
    void setLastChecked(const QDateTime &dateTime) { m_lastChecked = dateTime; }
    void setUsername(const QString &username) { m_username = username; }
    void setPassword(const QString &password) { m_password = password; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    // Utility methods
    QString getTypeString() const;
    QString getStatusString() const;
    QString getFreeSpaceString() const;
    double getFreeSpacePercentage() const;
    bool isValid() const;
    
private:
    QString m_id;
    QString m_path;
    DestinationType m_type;
    DestinationStatus m_status;
    qint64 m_freeSpace;      // in bytes
    qint64 m_totalSpace;     // in bytes
    QDateTime m_lastChecked;
    QString m_username;       // for network/cloud destinations
    QString m_password;       // encrypted in real implementation
    bool m_enabled;
    
    void generateId();
};

#endif // BACKUPDESTINATION_H
