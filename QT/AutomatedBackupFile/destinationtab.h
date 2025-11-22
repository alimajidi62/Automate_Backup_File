#ifndef DESTINATIONTAB_H
#define DESTINATIONTAB_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class DestinationTab;
}

class DestinationTab : public QWidget
{
    Q_OBJECT

public:
    explicit DestinationTab(QWidget *parent = nullptr);
    ~DestinationTab();
    
    // Public accessors
    QPushButton* getBtnBrowseDestination();
    QPushButton* getBtnAddCloudDest();
    QPushButton* getBtnRemoveDestination();

private:
    Ui::DestinationTab *ui;
};

#endif // DESTINATIONTAB_H
