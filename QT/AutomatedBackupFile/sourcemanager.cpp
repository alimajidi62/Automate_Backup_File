#include "sourcemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QTimer>
#include <QtConcurrent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "mpr.lib")
#endif

SourceManager::SourceManager(QObject *parent)
    : QObject(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_changeMonitoringEnabled(false)
    , m_checkIntervalMinutes(60)
    , m_checkTimer(new QTimer(this))
{
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &SourceManager::onFileSystemChanged);
    
    connect(m_checkTimer, &QTimer::timeout,
            this, &SourceManager::onCheckTimerTimeout);
}

SourceManager::~SourceManager()
{
    qDeleteAll(m_sources);
    m_sources.clear();
}

bool SourceManager::addSource(BackupSource *source)
{
    if (!source || !validateSource(source)) {
        emit error("Invalid source");
        return false;
    }

    // Check for duplicates
    for (const auto *existing : m_sources) {
        if (existing->getPath() == source->getPath()) {
            emit error("Source already exists: " + source->getPath());
            return false;
        }
    }

    m_sources.append(source);
    
    // Add to file watcher if monitoring is enabled
    if (m_changeMonitoringEnabled && source->getType() == SourceType::Local) {
        m_fileWatcher->addPath(source->getPath());
    }

    emit sourceAdded(source->getId());
    
    // Automatically check the source
    checkSource(source);
    
    return true;
}

bool SourceManager::removeSource(const QString &sourceId)
{
    for (int i = 0; i < m_sources.count(); ++i) {
        if (m_sources[i]->getId() == sourceId) {
            BackupSource *source = m_sources[i];
            
            // Remove from file watcher
            if (m_changeMonitoringEnabled) {
                m_fileWatcher->removePath(source->getPath());
            }
            
            m_sources.removeAt(i);
            emit sourceRemoved(sourceId);
            delete source;
            return true;
        }
    }
    
    emit error("Source not found");
    return false;
}

BackupSource* SourceManager::getSource(const QString &sourceId) const
{
    for (auto *source : m_sources) {
        if (source->getId() == sourceId) {
            return source;
        }
    }
    return nullptr;
}

QList<BackupSource*> SourceManager::getEnabledSources() const
{
    QList<BackupSource*> enabled;
    for (auto *source : m_sources) {
        if (source->isEnabled()) {
            enabled.append(source);
        }
    }
    return enabled;
}

void SourceManager::checkSource(BackupSource *source)
{
    if (!source) {
        return;
    }

    QString sourceId = source->getId();
    source->setStatus(SourceStatus::Checking);
    emit sourceStatusChanged(sourceId, SourceStatus::Checking);

    // Run check asynchronously (ignore return value as we don't need the QFuture)
    (void)QtConcurrent::run([this, source, sourceId]() {
        bool success = false;

        switch (source->getType()) {
            case SourceType::Local:
                checkLocalSource(source);
                success = source->getStatus() == SourceStatus::Available;
                break;
            case SourceType::Network:
                checkNetworkSource(source);
                success = source->getStatus() == SourceStatus::Available;
                break;
            case SourceType::Cloud:
                checkCloudSource(source);
                success = source->getStatus() == SourceStatus::Available;
                break;
        }

        source->setLastChecked(QDateTime::currentDateTime());
        emit sourceStatusChanged(sourceId, source->getStatus());
        emit sourceUpdated(sourceId);
        emit sourceCheckCompleted(sourceId, success);
    });
}

void SourceManager::checkAllSources()
{
    for (auto *source : m_sources) {
        if (source->isEnabled()) {
            checkSource(source);
        }
    }
}

bool SourceManager::testLocalPath(const QString &path)
{
    QFileInfo info(path);
    return info.exists() && info.isDir() && info.isReadable();
}

bool SourceManager::testNetworkPath(const QString &path, const QString &username, 
                                   const QString &password, const QString &domain)
{
#ifdef Q_OS_WIN
    // For Windows, try to access network path
    if (!username.isEmpty() && !password.isEmpty()) {
        // Try to connect with credentials
        NETRESOURCEW netResource;
        memset(&netResource, 0, sizeof(netResource));
        netResource.dwType = RESOURCETYPE_DISK;
        netResource.lpRemoteName = (LPWSTR)path.toStdWString().c_str();

        QString userWithDomain = domain.isEmpty() ? username : domain + "\\" + username;
        
        DWORD result = WNetAddConnection2W(&netResource, 
                                           (LPCWSTR)password.toStdWString().c_str(),
                                           (LPCWSTR)userWithDomain.toStdWString().c_str(),
                                           CONNECT_TEMPORARY);
        
        if (result == NO_ERROR || result == ERROR_SESSION_CREDENTIAL_CONFLICT) {
            // Connection successful or already connected
            QFileInfo info(path);
            return info.exists() && info.isDir();
        }
        return false;
    }
#endif

    // Try without credentials
    QFileInfo info(path);
    return info.exists() && info.isDir() && info.isReadable();
}

bool SourceManager::testCloudPath(const QString &path)
{
    // Cloud path testing would require cloud provider integration
    // For now, just return true as placeholder
    Q_UNUSED(path);
    return true;
}

void SourceManager::enableChangeMonitoring(bool enable)
{
    m_changeMonitoringEnabled = enable;
    
    if (enable) {
        // Add all local sources to watcher
        for (auto *source : m_sources) {
            if (source->getType() == SourceType::Local && source->isEnabled()) {
                m_fileWatcher->addPath(source->getPath());
            }
        }
        
        // Start periodic check timer
        m_checkTimer->start(m_checkIntervalMinutes * 60 * 1000);
    } else {
        // Remove all paths from watcher
        if (!m_fileWatcher->directories().isEmpty()) {
            m_fileWatcher->removePaths(m_fileWatcher->directories());
        }
        
        // Stop timer
        m_checkTimer->stop();
    }
}

qint64 SourceManager::getTotalSourceSize() const
{
    qint64 total = 0;
    for (const auto *source : m_sources) {
        if (source->isEnabled() && source->getStatus() == SourceStatus::Available) {
            total += source->getTotalSize();
        }
    }
    return total;
}

int SourceManager::getTotalFileCount() const
{
    int total = 0;
    for (const auto *source : m_sources) {
        if (source->isEnabled() && source->getStatus() == SourceStatus::Available) {
            total += source->getFileCount();
        }
    }
    return total;
}

bool SourceManager::saveToFile(const QString &filePath)
{
    QJsonObject root;
    root["version"] = "1.0";
    root["changeMonitoringEnabled"] = m_changeMonitoringEnabled;
    root["checkIntervalMinutes"] = m_checkIntervalMinutes;
    
    QJsonArray sourcesArray;
    for (const auto *source : m_sources) {
        sourcesArray.append(source->toJson());
    }
    root["sources"] = sourcesArray;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Failed to save sources: " + file.errorString());
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    return true;
}

bool SourceManager::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        // File might not exist yet, not an error
        return true;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        emit error("Invalid sources file format");
        return false;
    }
    
    QJsonObject root = doc.object();
    m_changeMonitoringEnabled = root["changeMonitoringEnabled"].toBool();
    m_checkIntervalMinutes = root["checkIntervalMinutes"].toInt(60);
    
    QJsonArray sourcesArray = root["sources"].toArray();
    for (const QJsonValue &value : sourcesArray) {
        BackupSource *source = BackupSource::fromJson(value.toObject());
        if (source) {
            m_sources.append(source);
            emit sourceAdded(source->getId());
        }
    }
    
    // Reapply monitoring if it was enabled
    if (m_changeMonitoringEnabled) {
        enableChangeMonitoring(true);
    }
    
    return true;
}

void SourceManager::onFileSystemChanged(const QString &path)
{
    // Find source with this path
    for (auto *source : m_sources) {
        if (source->getPath() == path) {
            emit sourceChanged(source->getId(), path);
            // Recalculate statistics
            calculateSourceStats(source);
            emit sourceUpdated(source->getId());
            break;
        }
    }
}

void SourceManager::onCheckTimerTimeout()
{
    // Periodically check all sources
    checkAllSources();
}

void SourceManager::checkLocalSource(BackupSource *source)
{
    if (!source) {
        return;
    }
    
    QFileInfo info(source->getPath());
    if (info.exists() && info.isDir() && info.isReadable()) {
        source->setStatus(SourceStatus::Available);
        source->setLastError("");
        calculateSourceStats(source);
    } else {
        source->setStatus(SourceStatus::Unavailable);
        source->setLastError("Directory not accessible");
    }
}

void SourceManager::checkNetworkSource(BackupSource *source)
{
    if (!source) {
        return;
    }
    
    bool success = false;
    
    if (source->requiresAuthentication()) {
        if (source->getUsername().isEmpty()) {
            source->setStatus(SourceStatus::CredentialsRequired);
            source->setLastError("Credentials required");
            return;
        }
        
        success = testNetworkPath(source->getPath(), 
                                  source->getUsername(),
                                  source->getPassword(),
                                  source->getDomain());
    } else {
        success = testNetworkPath(source->getPath());
    }
    
    if (success) {
        source->setStatus(SourceStatus::Available);
        source->setLastError("");
        calculateSourceStats(source);
    } else {
        source->setStatus(SourceStatus::Unavailable);
        source->setLastError("Network path not accessible");
    }
}

void SourceManager::checkCloudSource(BackupSource *source)
{
    if (!source) {
        return;
    }
    
    // Cloud checking would require cloud provider integration
    // For now, just mark as available
    source->setStatus(SourceStatus::Available);
    source->setLastError("");
}

bool SourceManager::validateSource(BackupSource *source) const
{
    if (!source) {
        return false;
    }
    
    return source->isValid();
}

void SourceManager::calculateSourceStats(BackupSource *source)
{
    if (!source || source->getType() != SourceType::Local) {
        return;
    }
    
    qint64 totalSize = 0;
    int fileCount = 0;
    
    QDirIterator it(source->getPath(), QDir::Files | QDir::NoDotAndDotDot, 
                   QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        totalSize += fileInfo.size();
        fileCount++;
    }
    
    source->setTotalSize(totalSize);
    source->setFileCount(fileCount);
}
