#ifndef SCHEDULETAB_H
#define SCHEDULETAB_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include "schedulemanager.h"

namespace Ui {
class ScheduleTab;
}

class ScheduleTab : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleTab(QWidget *parent = nullptr);
    ~ScheduleTab();
    
    // Public accessors
    QPushButton* getBtnAddSchedule();
    QPushButton* getBtnEditSchedule();
    QPushButton* getBtnRemoveSchedule();
    QCheckBox* getChkEnableScheduler();
    ScheduleManager* getScheduleManager() { return m_scheduleManager; }

private slots:
    void onAddSchedule();
    void onEditSchedule();
    void onRemoveSchedule();
    void onEnableScheduler(bool enabled);
    void onQuickSchedule();
    void onScheduleAdded(const QString &scheduleId);
    void onScheduleRemoved(const QString &scheduleId);
    void onScheduleUpdated(const QString &scheduleId);
    void onTableSelectionChanged();

private:
    Ui::ScheduleTab *ui;
    ScheduleManager *m_scheduleManager;
    
    void setupConnections();
    void refreshTable();
    void updateScheduleRow(int row, BackupSchedule *schedule);
    BackupSchedule* getSelectedSchedule() const;
    void showScheduleDialog(BackupSchedule *schedule = nullptr);
};

#endif // SCHEDULETAB_H
