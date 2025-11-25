#ifndef DESTINATIONTAB_H
#define DESTINATIONTAB_H

#include <QWidget>
#include <QPushButton>
#include "destinationmanager.h"
#include "backupfilemonitor.h"
#include "filedecryptor.h"

namespace Ui {
class DestinationTab;
}

class DestinationTab : public QWidget
{
    Q_OBJECT

public:
    explicit DestinationTab(QWidget *parent = nullptr);
    ~DestinationTab();
    
    // Public accessors
    QPushButton* getBtnBrowseDestination();
    QPushButton* getBtnAddCloudDest();
    QPushButton* getBtnRemoveDestination();
    
    // Backend access
    DestinationManager* getDestinationManager() { return m_destinationManager; }
    BackupFileMonitor* getBackupFileMonitor() { return m_backupFileMonitor; }

private slots:
    void onAddLocalDestination();
    void onAddCloudDestination();
    void onRemoveDestination();
    void onCheckDestination();
    void onRefreshDestinations();
    void onRetentionDaysChanged(int days);
    void onAutoCleanupToggled(bool enabled);
    
    // Manager signals
    void onDestinationAdded(const QString &destinationId);
    void onDestinationRemoved(const QString &destinationId);
    void onDestinationUpdated(const QString &destinationId);
    void onDestinationStatusChanged(const QString &destinationId, DestinationStatus status);
    void onCheckCompleted(const QString &destinationId, bool success);
    void onError(const QString &message);
    
    // File monitor slots
    void onFileAdded(const QString &destinationId, const QString &filePath, const BackupFileInfo &info);
    void onFileModified(const QString &destinationId, const QString &filePath, 
                        const BackupFileInfo &oldInfo, const BackupFileInfo &newInfo);
    void onFileDeleted(const QString &destinationId, const QString &filePath, const BackupFileInfo &info);
    void onScanCompleted(const QString &destinationId, int filesFound, int changesDetected);
    void onChangeDetected(const QString &destinationId, const FileChangeRecord &change);
    void onMonitoringStateChanged(bool enabled);
    void onViewChangeHistory();
    void onToggleMonitoring(bool enabled);
    void onDecryptBackup();

private:
    Ui::DestinationTab *ui;
    DestinationManager *m_destinationManager;
    BackupFileMonitor *m_backupFileMonitor;
    
    void setupConnections();
    void setupFileMonitorConnections();
    void refreshDestinationTable();
    void updateRetentionPolicy();
    void updateMonitoringStatus();
    QString getSelectedDestinationId() const;
    QString formatBytes(qint64 bytes) const;
};

#endif // DESTINATIONTAB_H
