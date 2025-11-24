#ifndef SOURCEMANAGER_H
#define SOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFileSystemWatcher>
#include <QTimer>
#include "backupsource.h"

class SourceManager : public QObject
{
    Q_OBJECT

public:
    explicit SourceManager(QObject *parent = nullptr);
    ~SourceManager();

    // Source management
    bool addSource(BackupSource *source);
    bool removeSource(const QString &sourceId);
    BackupSource* getSource(const QString &sourceId) const;
    QList<BackupSource*> getAllSources() const { return m_sources; }
    QList<BackupSource*> getEnabledSources() const;
    int getSourceCount() const { return m_sources.count(); }

    // Connectivity testing
    void checkSource(BackupSource *source);
    void checkAllSources();
    bool testLocalPath(const QString &path);
    bool testNetworkPath(const QString &path, const QString &username = QString(), 
                        const QString &password = QString(), const QString &domain = QString());
    bool testCloudPath(const QString &path);

    // Change monitoring
    void enableChangeMonitoring(bool enable);
    bool isChangeMonitoringEnabled() const { return m_changeMonitoringEnabled; }
    void setCheckInterval(int minutes) { m_checkIntervalMinutes = minutes; }
    int getCheckInterval() const { return m_checkIntervalMinutes; }

    // Statistics
    qint64 getTotalSourceSize() const;
    int getTotalFileCount() const;

    // Persistence
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

signals:
    void sourceAdded(const QString &sourceId);
    void sourceRemoved(const QString &sourceId);
    void sourceUpdated(const QString &sourceId);
    void sourceStatusChanged(const QString &sourceId, SourceStatus status);
    void sourceCheckCompleted(const QString &sourceId, bool success);
    void sourceChanged(const QString &sourceId, const QString &path);
    void error(const QString &errorMessage);

private slots:
    void onFileSystemChanged(const QString &path);
    void onCheckTimerTimeout();

private:
    QList<BackupSource*> m_sources;
    QFileSystemWatcher *m_fileWatcher;
    bool m_changeMonitoringEnabled;
    int m_checkIntervalMinutes;
    QTimer *m_checkTimer;

    void checkLocalSource(BackupSource *source);
    void checkNetworkSource(BackupSource *source);
    void checkCloudSource(BackupSource *source);
    bool validateSource(BackupSource *source) const;
    void calculateSourceStats(BackupSource *source);
};

#endif // SOURCEMANAGER_H
