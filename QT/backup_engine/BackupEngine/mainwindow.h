#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "backupengine.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_StartBackup_clicked();
    void on_StopBackup_clicked();

private:
    Ui::MainWindow *ui;
    BackupEngine *m_backupEngine;
};
#endif // MAINWINDOW_H
