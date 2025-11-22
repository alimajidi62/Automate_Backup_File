#include "sourcestab.h"
#include "ui_sourcestab.h"

SourcesTab::SourcesTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SourcesTab)
{
    ui->setupUi(this);
}

SourcesTab::~SourcesTab()
{
    delete ui;
}

QPushButton* SourcesTab::getBtnAddNetwork() { return ui->btnAddNetwork; }
QPushButton* SourcesTab::getBtnAddCloud() { return ui->btnAddCloud; }
QPushButton* SourcesTab::getBtnEditSource() { return ui->btnEditSource; }
QPushButton* SourcesTab::getBtnRemoveSource() { return ui->btnRemoveSource; }
QPushButton* SourcesTab::getBtnTestConnection() { return ui->btnTestConnection; }
