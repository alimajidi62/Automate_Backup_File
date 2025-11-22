#include "settingstab.h"
#include "ui_settingstab.h"

SettingsTab::SettingsTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsTab)
{
    ui->setupUi(this);
}

SettingsTab::~SettingsTab()
{
    delete ui;
}

QPushButton* SettingsTab::getBtnSaveSettings() { return ui->btnSaveSettings; }
QPushButton* SettingsTab::getBtnTestEncryption() { return ui->btnTestEncryption; }
