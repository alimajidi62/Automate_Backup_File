#ifndef TASKSTAB_H
#define TASKSTAB_H

#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>

namespace Ui {
class TasksTab;
}

class TasksTab : public QWidget
{
    Q_OBJECT

public:
    explicit TasksTab(QWidget *parent = nullptr);
    ~TasksTab();
    
    // Public accessors
    QPushButton* getBtnStartBackup();
    QPushButton* getBtnStopBackup();
    QPushButton* getBtnViewHistory();
    QProgressBar* getProgressBar();
    QLabel* getStatusLabel();

private:
    Ui::TasksTab *ui;
};

#endif // TASKSTAB_H
