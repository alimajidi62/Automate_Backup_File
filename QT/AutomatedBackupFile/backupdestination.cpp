#include "backupdestination.h"
#include <QUuid>
#include <QStorageInfo>

BackupDestination::BackupDestination()
    : m_type(DestinationType::Local)
    , m_status(DestinationStatus::Unavailable)
    , m_freeSpace(0)
    , m_totalSpace(0)
    , m_enabled(true)
{
    generateId();
}

BackupDestination::BackupDestination(const QString &path, DestinationType type)
    : m_path(path)
    , m_type(type)
    , m_status(DestinationStatus::Unavailable)
    , m_freeSpace(0)
    , m_totalSpace(0)
    , m_enabled(true)
{
    generateId();
}

void BackupDestination::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString BackupDestination::getTypeString() const
{
    switch (m_type) {
        case DestinationType::Local:
            return "Local";
        case DestinationType::Network:
            return "Network";
        case DestinationType::Cloud:
            return "Cloud";
        default:
            return "Unknown";
    }
}

QString BackupDestination::getStatusString() const
{
    switch (m_status) {
        case DestinationStatus::Available:
            return "Available";
        case DestinationStatus::Unavailable:
            return "Unavailable";
        case DestinationStatus::Checking:
            return "Checking...";
        case DestinationStatus::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

QString BackupDestination::getFreeSpaceString() const
{
    if (m_freeSpace <= 0) {
        return "Unknown";
    }
    
    double size = m_freeSpace;
    QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

double BackupDestination::getFreeSpacePercentage() const
{
    if (m_totalSpace <= 0) {
        return 0.0;
    }
    return (static_cast<double>(m_freeSpace) / static_cast<double>(m_totalSpace)) * 100.0;
}

bool BackupDestination::isValid() const
{
    return !m_path.isEmpty() && m_status != DestinationStatus::Error;
}
