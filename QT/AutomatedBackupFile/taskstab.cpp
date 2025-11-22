#include "taskstab.h"
#include "ui_taskstab.h"

TasksTab::TasksTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TasksTab)
{
    ui->setupUi(this);
}

TasksTab::~TasksTab()
{
    delete ui;
}

QPushButton* TasksTab::getBtnStartBackup() { return ui->btnStartBackup; }
QPushButton* TasksTab::getBtnStopBackup() { return ui->btnStopBackup; }
QPushButton* TasksTab::getBtnViewHistory() { return ui->btnViewHistory; }
