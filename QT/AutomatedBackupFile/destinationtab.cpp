#include "destinationtab.h"
#include "ui_destinationtab.h"

DestinationTab::DestinationTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DestinationTab)
{
    ui->setupUi(this);
}

DestinationTab::~DestinationTab()
{
    delete ui;
}

QPushButton* DestinationTab::getBtnBrowseDestination() { return ui->btnBrowseDestination; }
QPushButton* DestinationTab::getBtnAddCloudDest() { return ui->btnAddCloudDest; }
QPushButton* DestinationTab::getBtnRemoveDestination() { return ui->btnRemoveDestination; }
