#ifndef SOURCESTAB_H
#define SOURCESTAB_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QCheckBox>
#include <QSpinBox>
#include "backupfilemonitor.h"

class SourceManager;

namespace Ui {
class SourcesTab;
}

class SourcesTab : public QWidget
{
    Q_OBJECT

public:
    explicit SourcesTab(QWidget *parent = nullptr);
    ~SourcesTab();
    
    // Public accessors for widgets
    QPushButton* getBtnAddLocal();
    QPushButton* getBtnAddNetwork();
    QPushButton* getBtnAddCloud();
    QPushButton* getBtnEditSource();
    QPushButton* getBtnRemoveSource();
    QPushButton* getBtnTestConnection();
    
    // Backend access
    SourceManager* getSourceManager() { return m_sourceManager; }
    BackupFileMonitor* getSourceFileMonitor() { return m_sourceFileMonitor; }

public slots:
    void onAddLocalSource();
    void onAddNetworkSource();
    void onAddCloudSource();
    void onEditSource();
    void onRemoveSource();
    void onTestConnection();
    
    // Source file monitor slots
    void onSourceFileAdded(const QString &sourceId, const QString &filePath, const BackupFileInfo &info);
    void onSourceFileModified(const QString &sourceId, const QString &filePath, 
                              const BackupFileInfo &oldInfo, const BackupFileInfo &newInfo);
    void onSourceFileDeleted(const QString &sourceId, const QString &filePath, const BackupFileInfo &info);
    void onSourceScanCompleted(const QString &sourceId, int filesFound, int changesDetected);
    void onSourceChangeDetected(const QString &sourceId, const FileChangeRecord &change);
    void onSourceMonitoringStateChanged(bool enabled);
    void onViewSourceChangeHistory();
    void onToggleSourceMonitoring(bool enabled);

private:
    void setupConnections();
    void setupSourceFileMonitorConnections();
    void refreshSourceTable();
    void updateSourceMonitoringStatus();
    QString getSelectedSourceId() const;
    QString formatBytes(qint64 bytes) const;

    Ui::SourcesTab *ui;
    SourceManager *m_sourceManager;
    BackupFileMonitor *m_sourceFileMonitor;
};

#endif // SOURCESTAB_H
