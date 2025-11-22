#ifndef BACKUPSOURCE_H
#define BACKUPSOURCE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

enum class SourceType {
    Local,
    Network,
    Cloud
};

enum class SourceStatus {
    Available,
    Unavailable,
    Checking,
    Error,
    CredentialsRequired
};

class BackupSource
{
public:
    BackupSource(const QString &path = QString(), SourceType type = SourceType::Local);
    ~BackupSource();

    // Getters
    QString getId() const { return m_id; }
    QString getPath() const { return m_path; }
    SourceType getType() const { return m_type; }
    SourceStatus getStatus() const { return m_status; }
    QString getUsername() const { return m_username; }
    QString getPassword() const { return m_password; }
    QString getDomain() const { return m_domain; }
    QDateTime getLastChecked() const { return m_lastChecked; }
    QString getLastError() const { return m_lastError; }
    bool isEnabled() const { return m_enabled; }
    bool requiresAuthentication() const { return m_requiresAuth; }
    qint64 getTotalSize() const { return m_totalSize; }
    int getFileCount() const { return m_fileCount; }

    // Setters
    void setPath(const QString &path) { m_path = path; }
    void setType(SourceType type) { m_type = type; }
    void setStatus(SourceStatus status) { m_status = status; }
    void setUsername(const QString &username) { m_username = username; }
    void setPassword(const QString &password) { m_password = password; }
    void setDomain(const QString &domain) { m_domain = domain; }
    void setLastChecked(const QDateTime &dateTime) { m_lastChecked = dateTime; }
    void setLastError(const QString &error) { m_lastError = error; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    void setRequiresAuthentication(bool required) { m_requiresAuth = required; }
    void setTotalSize(qint64 size) { m_totalSize = size; }
    void setFileCount(int count) { m_fileCount = count; }

    // Utility methods
    QString getTypeString() const;
    QString getStatusString() const;
    bool isValid() const;
    QString getDisplayPath() const;

    // Serialization
    QJsonObject toJson() const;
    static BackupSource* fromJson(const QJsonObject &json);

private:
    QString m_id;
    QString m_path;
    SourceType m_type;
    SourceStatus m_status;
    QString m_username;
    QString m_password;
    QString m_domain;
    QDateTime m_lastChecked;
    QString m_lastError;
    bool m_enabled;
    bool m_requiresAuth;
    qint64 m_totalSize;
    int m_fileCount;

    void generateId();
};

#endif // BACKUPSOURCE_H
