#include "scheduletab.h"
#include "ui_scheduletab.h"

ScheduleTab::ScheduleTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScheduleTab)
{
    ui->setupUi(this);
}

ScheduleTab::~ScheduleTab()
{
    delete ui;
}

QPushButton* ScheduleTab::getBtnAddSchedule() { return ui->btnAddSchedule; }
QPushButton* ScheduleTab::getBtnEditSchedule() { return ui->btnEditSchedule; }
QPushButton* ScheduleTab::getBtnRemoveSchedule() { return ui->btnRemoveSchedule; }
QCheckBox* ScheduleTab::getChkEnableScheduler() { return ui->chkEnableScheduler; }
