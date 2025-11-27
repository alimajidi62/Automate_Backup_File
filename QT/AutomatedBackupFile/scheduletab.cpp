#include "scheduletab.h"
#include "ui_scheduletab.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTimeEdit>
#include <QCheckBox>
#include <QHeaderView>

ScheduleTab::ScheduleTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScheduleTab)
    , m_scheduleManager(new ScheduleManager(this))
{
    ui->setupUi(this);
    
    // Setup table headers
    ui->tableSchedules->setColumnCount(5);
    ui->tableSchedules->setHorizontalHeaderLabels({"Schedule Name", "Frequency", "Time", "Next Run", "Enabled"});
    ui->tableSchedules->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableSchedules->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableSchedules->horizontalHeader()->setStretchLastSection(false);
    ui->tableSchedules->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableSchedules->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableSchedules->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableSchedules->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableSchedules->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    setupConnections();
    
    // Load saved schedules
    m_scheduleManager->loadFromFile("schedules.json");
    refreshTable();
    
    // Restore scheduler state
    ui->chkEnableScheduler->setChecked(m_scheduleManager->isSchedulerEnabled());
}

ScheduleTab::~ScheduleTab()
{
    // Save schedules before cleanup
    m_scheduleManager->saveToFile("schedules.json");
    delete ui;
}

QPushButton* ScheduleTab::getBtnAddSchedule() { return ui->btnAddSchedule; }
QPushButton* ScheduleTab::getBtnEditSchedule() { return ui->btnEditSchedule; }
QPushButton* ScheduleTab::getBtnRemoveSchedule() { return ui->btnRemoveSchedule; }
QCheckBox* ScheduleTab::getChkEnableScheduler() { return ui->chkEnableScheduler; }

void ScheduleTab::setupConnections()
{
    connect(ui->btnAddSchedule, &QPushButton::clicked, this, &ScheduleTab::onAddSchedule);
    connect(ui->btnEditSchedule, &QPushButton::clicked, this, &ScheduleTab::onEditSchedule);
    connect(ui->btnRemoveSchedule, &QPushButton::clicked, this, &ScheduleTab::onRemoveSchedule);
    connect(ui->chkEnableScheduler, &QCheckBox::toggled, this, &ScheduleTab::onEnableScheduler);
    connect(ui->btnQuickSchedule, &QPushButton::clicked, this, &ScheduleTab::onQuickSchedule);
    
    connect(m_scheduleManager, &ScheduleManager::scheduleAdded, this, &ScheduleTab::onScheduleAdded);
    connect(m_scheduleManager, &ScheduleManager::scheduleRemoved, this, &ScheduleTab::onScheduleRemoved);
    connect(m_scheduleManager, &ScheduleManager::scheduleUpdated, this, &ScheduleTab::onScheduleUpdated);
    
    connect(ui->tableSchedules, &QTableWidget::itemSelectionChanged, this, &ScheduleTab::onTableSelectionChanged);
}

void ScheduleTab::onAddSchedule()
{
    showScheduleDialog();
}

void ScheduleTab::onEditSchedule()
{
    BackupSchedule *schedule = getSelectedSchedule();
    if (schedule) {
        showScheduleDialog(schedule);
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a schedule to edit.");
    }
}

void ScheduleTab::onRemoveSchedule()
{
    BackupSchedule *schedule = getSelectedSchedule();
    if (!schedule) {
        QMessageBox::warning(this, "No Selection", "Please select a schedule to remove.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Removal",
        "Are you sure you want to remove the schedule '" + schedule->getName() + "'?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_scheduleManager->removeSchedule(schedule->getId())) {
            m_scheduleManager->saveToFile("schedules.json");
        }
    }
}

void ScheduleTab::onEnableScheduler(bool enabled)
{
    m_scheduleManager->setSchedulerEnabled(enabled);
    m_scheduleManager->saveToFile("schedules.json");
    
    if (enabled) {
        ui->lblStatus->setText("Scheduler is running");
    } else {
        ui->lblStatus->setText("Scheduler is stopped");
    }
}

void ScheduleTab::onQuickSchedule()
{
    QString name = ui->lineScheduleName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a schedule name.");
        return;
    }
    
    ScheduleFrequency frequency = static_cast<ScheduleFrequency>(ui->comboFrequency->currentIndex());
    QTime time = ui->timeSchedule->time();
    
    auto *schedule = new BackupSchedule(name, frequency, time);
    
    if (m_scheduleManager->addSchedule(schedule)) {
        m_scheduleManager->saveToFile("schedules.json");
        ui->lineScheduleName->clear();
    } else {
        delete schedule;
        QMessageBox::warning(this, "Error", "Failed to add schedule. Check if a schedule with this name already exists.");
    }
}

void ScheduleTab::onScheduleAdded(const QString &scheduleId)
{
    Q_UNUSED(scheduleId);
    refreshTable();
}

void ScheduleTab::onScheduleRemoved(const QString &scheduleId)
{
    Q_UNUSED(scheduleId);
    refreshTable();
}

void ScheduleTab::onScheduleUpdated(const QString &scheduleId)
{
    // Find the row for this schedule and update it
    BackupSchedule *schedule = m_scheduleManager->getSchedule(scheduleId);
    if (!schedule) return;
    
    for (int row = 0; row < ui->tableSchedules->rowCount(); ++row) {
        QTableWidgetItem *item = ui->tableSchedules->item(row, 0);
        if (item && item->data(Qt::UserRole).toString() == scheduleId) {
            updateScheduleRow(row, schedule);
            break;
        }
    }
}

void ScheduleTab::onTableSelectionChanged()
{
    bool hasSelection = !ui->tableSchedules->selectedItems().isEmpty();
    ui->btnEditSchedule->setEnabled(hasSelection);
    ui->btnRemoveSchedule->setEnabled(hasSelection);
}

void ScheduleTab::refreshTable()
{
    ui->tableSchedules->setRowCount(0);
    
    QList<BackupSchedule*> schedules = m_scheduleManager->getAllSchedules();
    for (BackupSchedule *schedule : schedules) {
        int row = ui->tableSchedules->rowCount();
        ui->tableSchedules->insertRow(row);
        updateScheduleRow(row, schedule);
    }
    
    onTableSelectionChanged();
}

void ScheduleTab::updateScheduleRow(int row, BackupSchedule *schedule)
{
    // Name
    QTableWidgetItem *nameItem = new QTableWidgetItem(schedule->getName());
    nameItem->setData(Qt::UserRole, schedule->getId());
    ui->tableSchedules->setItem(row, 0, nameItem);
    
    // Frequency
    ui->tableSchedules->setItem(row, 1, new QTableWidgetItem(schedule->getFrequencyString()));
    
    // Time
    ui->tableSchedules->setItem(row, 2, new QTableWidgetItem(schedule->getTime().toString("hh:mm")));
    
    // Next Run
    QString nextRunText = schedule->getNextRun().isValid() 
        ? schedule->getNextRun().toString("yyyy-MM-dd hh:mm") 
        : "Not scheduled";
    ui->tableSchedules->setItem(row, 3, new QTableWidgetItem(nextRunText));
    
    // Enabled
    QTableWidgetItem *enabledItem = new QTableWidgetItem(schedule->isEnabled() ? "Yes" : "No");
    enabledItem->setTextAlignment(Qt::AlignCenter);
    ui->tableSchedules->setItem(row, 4, enabledItem);
}

BackupSchedule* ScheduleTab::getSelectedSchedule() const
{
    QList<QTableWidgetItem*> selected = ui->tableSchedules->selectedItems();
    if (selected.isEmpty()) {
        return nullptr;
    }
    
    int row = selected.first()->row();
    QTableWidgetItem *item = ui->tableSchedules->item(row, 0);
    if (!item) return nullptr;
    
    QString scheduleId = item->data(Qt::UserRole).toString();
    return m_scheduleManager->getSchedule(scheduleId);
}

void ScheduleTab::showScheduleDialog(BackupSchedule *schedule)
{
    QDialog dialog(this);
    dialog.setWindowTitle(schedule ? "Edit Schedule" : "Add Schedule");
    dialog.resize(400, 300);
    
    QFormLayout *layout = new QFormLayout(&dialog);
    
    // Name
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    if (schedule) nameEdit->setText(schedule->getName());
    layout->addRow("Schedule Name:", nameEdit);
    
    // Frequency
    QComboBox *frequencyCombo = new QComboBox(&dialog);
    frequencyCombo->addItems({"Daily", "Weekly", "Monthly", "Custom"});
    if (schedule) frequencyCombo->setCurrentIndex(static_cast<int>(schedule->getFrequency()));
    layout->addRow("Frequency:", frequencyCombo);
    
    // Time
    QTimeEdit *timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("hh:mm");
    if (schedule) {
        timeEdit->setTime(schedule->getTime());
    } else {
        timeEdit->setTime(QTime(0, 0));
    }
    layout->addRow("Time:", timeEdit);
    
    // Weekdays (for weekly)
    QListWidget *weekdaysList = new QListWidget(&dialog);
    weekdaysList->setSelectionMode(QAbstractItemView::MultiSelection);
    QStringList dayNames = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (const QString &day : dayNames) {
        weekdaysList->addItem(day);
    }
    if (schedule && schedule->getFrequency() == ScheduleFrequency::Weekly) {
        for (DayOfWeek day : schedule->getWeekDays()) {
            weekdaysList->item(static_cast<int>(day) - 1)->setSelected(true);
        }
    }
    layout->addRow("Weekdays (Weekly):", weekdaysList);
    
    // Day of month (for monthly)
    QSpinBox *dayOfMonthSpin = new QSpinBox(&dialog);
    dayOfMonthSpin->setRange(1, 31);
    if (schedule) dayOfMonthSpin->setValue(schedule->getDayOfMonth());
    layout->addRow("Day of Month (Monthly):", dayOfMonthSpin);
    
    // Interval minutes (for custom)
    QSpinBox *intervalSpin = new QSpinBox(&dialog);
    intervalSpin->setRange(1, 1440);
    intervalSpin->setSuffix(" minutes");
    if (schedule) intervalSpin->setValue(schedule->getIntervalMinutes());
    layout->addRow("Interval (Custom):", intervalSpin);
    
    // Enabled checkbox
    QCheckBox *enabledCheck = new QCheckBox(&dialog);
    enabledCheck->setChecked(schedule ? schedule->isEnabled() : true);
    layout->addRow("Enabled:", enabledCheck);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addRow(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a schedule name.");
            return;
        }
        
        ScheduleFrequency frequency = static_cast<ScheduleFrequency>(frequencyCombo->currentIndex());
        QTime time = timeEdit->time();
        
        if (schedule) {
            // Update existing schedule
            schedule->setName(name);
            schedule->setFrequency(frequency);
            schedule->setTime(time);
            schedule->setEnabled(enabledCheck->isChecked());
            
            if (frequency == ScheduleFrequency::Weekly) {
                QList<DayOfWeek> weekdays;
                for (int i = 0; i < weekdaysList->count(); ++i) {
                    if (weekdaysList->item(i)->isSelected()) {
                        weekdays.append(static_cast<DayOfWeek>(i + 1));
                    }
                }
                schedule->setWeekDays(weekdays);
            } else if (frequency == ScheduleFrequency::Monthly) {
                schedule->setDayOfMonth(dayOfMonthSpin->value());
            } else if (frequency == ScheduleFrequency::Custom) {
                schedule->setIntervalMinutes(intervalSpin->value());
            }
            
            schedule->setNextRun(schedule->calculateNextRun());
            m_scheduleManager->saveToFile("schedules.json");
            refreshTable();
        } else {
            // Add new schedule
            auto *newSchedule = new BackupSchedule(name, frequency, time);
            newSchedule->setEnabled(enabledCheck->isChecked());
            
            if (frequency == ScheduleFrequency::Weekly) {
                QList<DayOfWeek> weekdays;
                for (int i = 0; i < weekdaysList->count(); ++i) {
                    if (weekdaysList->item(i)->isSelected()) {
                        weekdays.append(static_cast<DayOfWeek>(i + 1));
                    }
                }
                newSchedule->setWeekDays(weekdays);
            } else if (frequency == ScheduleFrequency::Monthly) {
                newSchedule->setDayOfMonth(dayOfMonthSpin->value());
            } else if (frequency == ScheduleFrequency::Custom) {
                newSchedule->setIntervalMinutes(intervalSpin->value());
            }
            
            if (m_scheduleManager->addSchedule(newSchedule)) {
                m_scheduleManager->saveToFile("schedules.json");
            } else {
                delete newSchedule;
                QMessageBox::warning(this, "Error", "Failed to add schedule.");
            }
        }
    }
}
