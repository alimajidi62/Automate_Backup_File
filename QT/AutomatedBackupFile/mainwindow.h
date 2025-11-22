#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SourcesTab;
class ScheduleTab;
class TasksTab;
class DestinationTab;
class SettingsTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Source Management
    void onAddNetworkPath();
    void onAddCloudPath();
    void onRemoveSource();
    void onEditSource();
    void onTestConnection();
    
    // Destination Management
    void onBrowseLocalDestination();
    void onAddCloudDestination();
    void onRemoveDestination();
    
    // Schedule Management
    void onAddSchedule();
    void onEditSchedule();
    void onRemoveSchedule();
    void onEnableSchedule(bool enabled);
    
    // Backup Operations
    void onStartBackup();
    void onStopBackup();
    void onViewBackupHistory();
    
    // Settings
    void onSaveSettings();
    void onTestEncryption();

private:
    Ui::MainWindow *ui;
    
    // Tab widgets
    SourcesTab *sourcesTab;
    ScheduleTab *scheduleTab;
    TasksTab *tasksTab;
    DestinationTab *destinationTab;
    SettingsTab *settingsTab;
    
    void setupConnections();
    void initializeUI();
};
#endif // MAINWINDOW_H
