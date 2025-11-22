#ifndef SOURCESTAB_H
#define SOURCESTAB_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QCheckBox>
#include <QSpinBox>

class SourceManager;

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
    QPushButton* getBtnAddLocal();
    QPushButton* getBtnAddNetwork();
    QPushButton* getBtnAddCloud();
    QPushButton* getBtnEditSource();
    QPushButton* getBtnRemoveSource();
    QPushButton* getBtnTestConnection();

public slots:
    void onAddLocalSource();
    void onAddNetworkSource();
    void onAddCloudSource();
    void onEditSource();
    void onRemoveSource();
    void onTestConnection();

private:
    void setupConnections();
    void refreshSourceTable();
    QString getSelectedSourceId() const;
    QString formatBytes(qint64 bytes) const;

    Ui::SourcesTab *ui;
    SourceManager *m_sourceManager;
};

#endif // SOURCESTAB_H
