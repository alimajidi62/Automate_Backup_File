#ifndef SOURCESTAB_H
#define SOURCESTAB_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QCheckBox>
#include <QSpinBox>

namespace Ui {
class SourcesTab;
}

class SourcesTab : public QWidget
{
    Q_OBJECT

public:
    explicit SourcesTab(QWidget *parent = nullptr);
    ~SourcesTab();
    
    // Public accessors for widgets
    QPushButton* getBtnAddNetwork();
    QPushButton* getBtnAddCloud();
    QPushButton* getBtnEditSource();
    QPushButton* getBtnRemoveSource();
    QPushButton* getBtnTestConnection();

private:
    Ui::SourcesTab *ui;
};

#endif // SOURCESTAB_H
