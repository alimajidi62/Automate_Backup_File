#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class SettingsTab;
}

class SettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsTab(QWidget *parent = nullptr);
    ~SettingsTab();
    
    // Public accessors
    QPushButton* getBtnSaveSettings();
    QPushButton* getBtnTestEncryption();

private:
    Ui::SettingsTab *ui;
};

#endif // SETTINGSTAB_H
