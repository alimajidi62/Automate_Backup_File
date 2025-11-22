#ifndef DESTINATIONTAB_H
#define DESTINATIONTAB_H

#include <QWidget>
#include <QPushButton>
#include "destinationmanager.h"

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
    
    // Backend access
    DestinationManager* getDestinationManager() { return m_destinationManager; }

private slots:
    void onAddLocalDestination();
    void onAddCloudDestination();
    void onRemoveDestination();
    void onCheckDestination();
    void onRefreshDestinations();
    void onRetentionDaysChanged(int days);
    void onAutoCleanupToggled(bool enabled);
    
    // Manager signals
    void onDestinationAdded(const QString &destinationId);
    void onDestinationRemoved(const QString &destinationId);
    void onDestinationUpdated(const QString &destinationId);
    void onDestinationStatusChanged(const QString &destinationId, DestinationStatus status);
    void onCheckCompleted(const QString &destinationId, bool success);
    void onError(const QString &message);

private:
    Ui::DestinationTab *ui;
    DestinationManager *m_destinationManager;
    
    void setupConnections();
    void refreshDestinationTable();
    void updateRetentionPolicy();
    QString getSelectedDestinationId() const;
};

#endif // DESTINATIONTAB_H
