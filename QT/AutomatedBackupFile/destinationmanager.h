#ifndef DESTINATIONMANAGER_H
#define DESTINATIONMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include "backupdestination.h"
#include "retentionpolicy.h"
#include "cloudprovider.h"

class DestinationManager : public QObject
{
    Q_OBJECT
    
public:
    explicit DestinationManager(QObject *parent = nullptr);
    ~DestinationManager();
    
    // Destination management
    bool addDestination(BackupDestination *destination);
    bool removeDestination(const QString &destinationId);
    BackupDestination* getDestination(const QString &destinationId) const;
    QList<BackupDestination*> getAllDestinations() const;
    int getDestinationCount() const;
    
    // Destination operations
    void checkDestination(const QString &destinationId);
    void checkAllDestinations();
    bool testConnection(BackupDestination *destination);
    void updateDestinationStatus(const QString &destinationId, DestinationStatus status);
    
    // Space management
    qint64 getTotalFreeSpace() const;
    qint64 getTotalUsedSpace() const;
    BackupDestination* findBestDestination(qint64 requiredSpace) const;
    
    // Retention policy
    void setRetentionPolicy(const RetentionPolicy &policy);
    RetentionPolicy getRetentionPolicy() const;
    void applyRetentionPolicy(const QString &destinationId);
    
    // Persistence
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);
    
    // Cloud Provider Management
    CloudProvider* getCloudProvider(const QString &destinationId);
    bool setCloudProvider(const QString &destinationId, CloudProvider *provider);
    QStringList getAvailableCloudProviders() const;

signals:
    void destinationAdded(const QString &destinationId);
    void destinationRemoved(const QString &destinationId);
    void destinationUpdated(const QString &destinationId);
    void destinationStatusChanged(const QString &destinationId, DestinationStatus status);
    void checkCompleted(const QString &destinationId, bool success);
    void error(const QString &message);
    
private slots:
    void onCheckCompleted();
    
private:
    QList<BackupDestination*> m_destinations;
    RetentionPolicy m_retentionPolicy;
    QMap<QString, CloudProvider*> m_cloudProviders; // Maps destination ID to cloud provider
    
    void checkLocalDestination(BackupDestination *destination);
    void checkNetworkDestination(BackupDestination *destination);
    void checkCloudDestination(BackupDestination *destination);
    bool validateDestination(BackupDestination *destination) const;
};

#endif // DESTINATIONMANAGER_H
