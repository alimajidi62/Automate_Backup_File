#include "backupsource.h"
#include <QUuid>
#include <QFileInfo>
#include <QDir>

BackupSource::BackupSource(const QString &path, SourceType type)
    : m_path(path)
    , m_type(type)
    , m_status(SourceStatus::Unavailable)
    , m_enabled(true)
    , m_requiresAuth(false)
    , m_totalSize(0)
    , m_fileCount(0)
{
    generateId();
}

BackupSource::~BackupSource()
{
}

void BackupSource::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString BackupSource::getTypeString() const
{
    switch (m_type) {
        case SourceType::Local:
            return "Local";
        case SourceType::Network:
            return "Network";
        case SourceType::Cloud:
            return "Cloud";
        default:
            return "Unknown";
    }
}

QString BackupSource::getStatusString() const
{
    switch (m_status) {
        case SourceStatus::Available:
            return "Available";
        case SourceStatus::Unavailable:
            return "Unavailable";
        case SourceStatus::Checking:
            return "Checking...";
        case SourceStatus::Error:
            return "Error";
        case SourceStatus::CredentialsRequired:
            return "Credentials Required";
        default:
            return "Unknown";
    }
}

bool BackupSource::isValid() const
{
    if (m_path.isEmpty()) {
        return false;
    }

    if (m_type == SourceType::Network && m_requiresAuth) {
        if (m_username.isEmpty()) {
            return false;
        }
    }

    return true;
}

QString BackupSource::getDisplayPath() const
{
    if (m_type == SourceType::Local) {
        QDir dir(m_path);
        return dir.absolutePath();
    }
    return m_path;
}

QJsonObject BackupSource::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["path"] = m_path;
    json["type"] = static_cast<int>(m_type);
    json["status"] = static_cast<int>(m_status);
    json["username"] = m_username;
    json["domain"] = m_domain;
    json["lastChecked"] = m_lastChecked.toString(Qt::ISODate);
    json["lastError"] = m_lastError;
    json["enabled"] = m_enabled;
    json["requiresAuth"] = m_requiresAuth;
    json["totalSize"] = QString::number(m_totalSize);
    json["fileCount"] = m_fileCount;
    // Note: Password is not saved for security reasons
    return json;
}

BackupSource* BackupSource::fromJson(const QJsonObject &json)
{
    BackupSource *source = new BackupSource();
    source->m_id = json["id"].toString();
    source->m_path = json["path"].toString();
    source->m_type = static_cast<SourceType>(json["type"].toInt());
    source->m_status = static_cast<SourceStatus>(json["status"].toInt());
    source->m_username = json["username"].toString();
    source->m_domain = json["domain"].toString();
    source->m_lastChecked = QDateTime::fromString(json["lastChecked"].toString(), Qt::ISODate);
    source->m_lastError = json["lastError"].toString();
    source->m_enabled = json["enabled"].toBool();
    source->m_requiresAuth = json["requiresAuth"].toBool();
    source->m_totalSize = json["totalSize"].toString().toLongLong();
    source->m_fileCount = json["fileCount"].toInt();
    return source;
}
