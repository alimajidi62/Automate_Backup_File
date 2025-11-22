#ifndef SCHEDULETAB_H
#define SCHEDULETAB_H

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>

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

private:
    Ui::ScheduleTab *ui;
};

#endif // SCHEDULETAB_H
