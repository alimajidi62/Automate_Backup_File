#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Load and apply stylesheet
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        QString stylesheet = stream.readAll();
        a.setStyleSheet(stylesheet);
        styleFile.close();
    }
    
    // Set application properties for better appearance
    a.setApplicationName("Automated Backup File Manager");
    a.setApplicationDisplayName("Automated Backup File Manager");
    a.setOrganizationName("BackupSolutions");
    
    MainWindow w;
    w.resize(1200, 800); // Set a nice default size
    w.show();
    return a.exec();
}
