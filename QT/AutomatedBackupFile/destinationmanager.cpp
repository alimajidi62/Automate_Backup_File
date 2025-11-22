#include "destinationmanager.h"
#include <QDir>
#include <QStorageInfo>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QtConcurrent>

DestinationManager::DestinationManager(QObject *parent)
    : QObject(parent)
{
}

DestinationManager::~DestinationManager()
{
    qDeleteAll(m_destinations);
    m_destinations.clear();
}

bool DestinationManager::addDestination(BackupDestination *destination)
{
    if (!destination || !validateDestination(destination)) {
        emit error("Invalid destination");
        return false;
    }
    
    // Check for duplicates
    for (const auto *existing : m_destinations) {
        if (existing->getPath() == destination->getPath()) {
            emit error("Destination already exists");
            return false;
        }
    }
    
    m_destinations.append(destination);
    emit destinationAdded(destination->getId());
    
    // Check destination status asynchronously
    checkDestination(destination->getId());
    
    return true;
}

bool DestinationManager::removeDestination(const QString &destinationId)
{
    for (int i = 0; i < m_destinations.size(); ++i) {
        if (m_destinations[i]->getId() == destinationId) {
            BackupDestination *dest = m_destinations.takeAt(i);
            emit destinationRemoved(destinationId);
            delete dest;
            return true;
        }
    }
    return false;
}

BackupDestination* DestinationManager::getDestination(const QString &destinationId) const
{
    for (auto *dest : m_destinations) {
        if (dest->getId() == destinationId) {
            return dest;
        }
    }
    return nullptr;
}

QList<BackupDestination*> DestinationManager::getAllDestinations() const
{
    return m_destinations;
}

int DestinationManager::getDestinationCount() const
{
    return m_destinations.size();
}

void DestinationManager::checkDestination(const QString &destinationId)
{
    BackupDestination *dest = getDestination(destinationId);
    if (!dest) {
        return;
    }
    
    dest->setStatus(DestinationStatus::Checking);
    emit destinationStatusChanged(destinationId, DestinationStatus::Checking);
    
    // Run check asynchronously (ignore return value as we don't need the QFuture)
    (void)QtConcurrent::run([this, dest, destinationId]() {
        bool success = false;
        
        switch (dest->getType()) {
            case DestinationType::Local:
                checkLocalDestination(dest);
                success = dest->getStatus() == DestinationStatus::Available;
                break;
            case DestinationType::Network:
                checkNetworkDestination(dest);
                success = dest->getStatus() == DestinationStatus::Available;
                break;
            case DestinationType::Cloud:
                checkCloudDestination(dest);
                success = dest->getStatus() == DestinationStatus::Available;
                break;
        }
        
        dest->setLastChecked(QDateTime::currentDateTime());
        
        // Emit signals on main thread
        QMetaObject::invokeMethod(this, [this, destinationId, success, dest]() {
            emit destinationStatusChanged(destinationId, dest->getStatus());
            emit checkCompleted(destinationId, success);
            emit destinationUpdated(destinationId);
        }, Qt::QueuedConnection);
    });
}

void DestinationManager::checkAllDestinations()
{
    for (const auto *dest : m_destinations) {
        checkDestination(dest->getId());
    }
}

bool DestinationManager::testConnection(BackupDestination *destination)
{
    if (!destination) {
        return false;
    }
    
    switch (destination->getType()) {
        case DestinationType::Local: {
            QDir dir(destination->getPath());
            return dir.exists();
        }
        case DestinationType::Network: {
            // Test network path accessibility
            QFileInfo info(destination->getPath());
            return info.exists() && info.isDir();
        }
        case DestinationType::Cloud: {
            // Cloud connection test would go here
            // For now, just return true as placeholder
            return true;
        }
    }
    
    return false;
}

void DestinationManager::updateDestinationStatus(const QString &destinationId, DestinationStatus status)
{
    BackupDestination *dest = getDestination(destinationId);
    if (dest) {
        dest->setStatus(status);
        emit destinationStatusChanged(destinationId, status);
        emit destinationUpdated(destinationId);
    }
}

qint64 DestinationManager::getTotalFreeSpace() const
{
    qint64 total = 0;
    for (const auto *dest : m_destinations) {
        if (dest->isEnabled() && dest->getStatus() == DestinationStatus::Available) {
            total += dest->getFreeSpace();
        }
    }
    return total;
}

qint64 DestinationManager::getTotalUsedSpace() const
{
    qint64 total = 0;
    for (const auto *dest : m_destinations) {
        if (dest->isEnabled() && dest->getStatus() == DestinationStatus::Available) {
            total += (dest->getTotalSpace() - dest->getFreeSpace());
        }
    }
    return total;
}

BackupDestination* DestinationManager::findBestDestination(qint64 requiredSpace) const
{
    BackupDestination *best = nullptr;
    qint64 maxFreeSpace = 0;
    
    for (auto *dest : m_destinations) {
        if (dest->isEnabled() && 
            dest->getStatus() == DestinationStatus::Available &&
            dest->getFreeSpace() >= requiredSpace &&
            dest->getFreeSpace() > maxFreeSpace) {
            best = dest;
            maxFreeSpace = dest->getFreeSpace();
        }
    }
    
    return best;
}

void DestinationManager::setRetentionPolicy(const RetentionPolicy &policy)
{
    m_retentionPolicy = policy;
}

RetentionPolicy DestinationManager::getRetentionPolicy() const
{
    return m_retentionPolicy;
}

void DestinationManager::applyRetentionPolicy(const QString &destinationId)
{
    // Placeholder for retention policy application
    // Would scan backup files and delete old ones based on policy
    BackupDestination *dest = getDestination(destinationId);
    if (!dest) {
        return;
    }
    
    // TODO: Implement actual file cleanup based on retention policy
}

bool DestinationManager::saveToFile(const QString &filePath)
{
    QJsonArray destinationsArray;
    
    for (const auto *dest : m_destinations) {
        QJsonObject obj;
        obj["id"] = dest->getId();
        obj["path"] = dest->getPath();
        obj["type"] = static_cast<int>(dest->getType());
        obj["enabled"] = dest->isEnabled();
        obj["username"] = dest->getUsername();
        // Note: In production, password should be encrypted
        obj["password"] = dest->getPassword();
        destinationsArray.append(obj);
    }
    
    QJsonObject root;
    root["destinations"] = destinationsArray;
    
    QJsonObject policyObj;
    policyObj["retentionDays"] = m_retentionPolicy.getRetentionDays();
    policyObj["autoCleanup"] = m_retentionPolicy.isAutoCleanupEnabled();
    policyObj["maxBackupCount"] = m_retentionPolicy.getMaxBackupCount();
    root["retentionPolicy"] = policyObj;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Failed to save destinations: " + file.errorString());
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    return true;
}

bool DestinationManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;  // File might not exist yet, not an error
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit error("Invalid configuration file format");
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Clear existing destinations
    qDeleteAll(m_destinations);
    m_destinations.clear();
    
    // Load destinations
    QJsonArray destinationsArray = root["destinations"].toArray();
    for (const QJsonValue &value : destinationsArray) {
        QJsonObject obj = value.toObject();
        
        auto *dest = new BackupDestination(
            obj["path"].toString(),
            static_cast<DestinationType>(obj["type"].toInt())
        );
        
        dest->setEnabled(obj["enabled"].toBool());
        dest->setUsername(obj["username"].toString());
        dest->setPassword(obj["password"].toString());
        
        m_destinations.append(dest);
    }
    
    // Load retention policy
    if (root.contains("retentionPolicy")) {
        QJsonObject policyObj = root["retentionPolicy"].toObject();
        m_retentionPolicy.setRetentionDays(policyObj["retentionDays"].toInt(30));
        m_retentionPolicy.setAutoCleanup(policyObj["autoCleanup"].toBool());
        m_retentionPolicy.setMaxBackupCount(policyObj["maxBackupCount"].toInt(0));
    }
    
    return true;
}

void DestinationManager::onCheckCompleted()
{
    // Handle check completion if needed
}

void DestinationManager::checkLocalDestination(BackupDestination *destination)
{
    QDir dir(destination->getPath());
    
    if (!dir.exists()) {
        destination->setStatus(DestinationStatus::Unavailable);
        return;
    }
    
    // Get storage information
    QStorageInfo storage(destination->getPath());
    
    if (storage.isValid() && storage.isReady()) {
        destination->setFreeSpace(storage.bytesAvailable());
        destination->setTotalSpace(storage.bytesTotal());
        destination->setStatus(DestinationStatus::Available);
    } else {
        destination->setStatus(DestinationStatus::Error);
    }
}

void DestinationManager::checkNetworkDestination(BackupDestination *destination)
{
    // Check if network path is accessible
    QFileInfo info(destination->getPath());
    
    if (!info.exists() || !info.isDir()) {
        destination->setStatus(DestinationStatus::Unavailable);
        return;
    }
    
    // Try to get storage info
    QStorageInfo storage(destination->getPath());
    
    if (storage.isValid() && storage.isReady()) {
        destination->setFreeSpace(storage.bytesAvailable());
        destination->setTotalSpace(storage.bytesTotal());
        destination->setStatus(DestinationStatus::Available);
    } else {
        destination->setStatus(DestinationStatus::Error);
    }
}

void DestinationManager::checkCloudDestination(BackupDestination *destination)
{
    if (!destination) {
        return;
    }
    
    // Get the cloud provider for this destination
    CloudProvider *provider = m_cloudProviders.value(destination->getId(), nullptr);
    
    if (!provider) {
        destination->setStatus(DestinationStatus::Error);
        destination->setFreeSpace(0);
        destination->setTotalSpace(0);
        emit error("No cloud provider configured for destination: " + destination->getPath());
        return;
    }
    
    // Test connection
    if (provider->testConnection()) {
        destination->setStatus(DestinationStatus::Available);
        
        // Get space information
        qint64 freeSpace = provider->getAvailableSpace();
        qint64 totalSpace = provider->getTotalSpace();
        
        destination->setFreeSpace(freeSpace);
        destination->setTotalSpace(totalSpace);
        destination->setLastChecked(QDateTime::currentDateTime());
    } else {
        destination->setStatus(DestinationStatus::Unavailable);
        destination->setFreeSpace(0);
        destination->setTotalSpace(0);
        emit error("Cloud connection failed: " + provider->getLastError());
    }
}

bool DestinationManager::validateDestination(BackupDestination *destination) const
{
    if (!destination) {
        return false;
    }
    
    if (destination->getPath().isEmpty()) {
        return false;
    }
    
    return true;
}

CloudProvider* DestinationManager::getCloudProvider(const QString &destinationId)
{
    return m_cloudProviders.value(destinationId, nullptr);
}

bool DestinationManager::setCloudProvider(const QString &destinationId, CloudProvider *provider)
{
    if (!provider) {
        return false;
    }
    
    // Remove old provider if exists
    if (m_cloudProviders.contains(destinationId)) {
        CloudProvider *oldProvider = m_cloudProviders[destinationId];
        if (oldProvider) {
            oldProvider->deleteLater();
        }
    }
    
    // Set the new provider
    m_cloudProviders[destinationId] = provider;
    provider->setParent(this);
    
    // Connect signals
    connect(provider, &CloudProvider::connectionStatusChanged,
            this, [this, destinationId](CloudProvider::ConnectionStatus status) {
        BackupDestination *dest = getDestination(destinationId);
        if (dest) {
            switch (status) {
                case CloudProvider::Connected:
                    dest->setStatus(DestinationStatus::Available);
                    break;
                case CloudProvider::Connecting:
                    dest->setStatus(DestinationStatus::Checking);
                    break;
                case CloudProvider::Disconnected:
                case CloudProvider::Error:
                    dest->setStatus(DestinationStatus::Unavailable);
                    break;
            }
            emit destinationUpdated(destinationId);
        }
    });
    
    connect(provider, &CloudProvider::error,
            this, &DestinationManager::error);
    
    return true;
}

QStringList DestinationManager::getAvailableCloudProviders() const
{
    return CloudProviderFactory::getAvailableProviders();
}
