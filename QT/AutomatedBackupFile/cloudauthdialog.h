#ifndef CLOUDAUTHDIALOG_H
#define CLOUDAUTHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QMap>

class QFormLayout;

class CloudAuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CloudAuthDialog(const QString &providerName, QWidget *parent = nullptr);
    ~CloudAuthDialog();

    QMap<QString, QString> getCredentials() const;
    bool isTestModeEnabled() const { return m_testMode; }

private slots:
    void onTestModeChanged(int state);
    void onOpenInstructionsLink();

private:
    void setupUI();
    void createGoogleDriveUI(QFormLayout *formLayout);
    void createDropboxUI(QFormLayout *formLayout);
    void createGenericUI(QFormLayout *formLayout);

    QString m_providerName;
    QMap<QString, QString> m_credentials;
    bool m_testMode;

    QLineEdit *m_accessTokenEdit;
    QLineEdit *m_clientIdEdit;
    QLineEdit *m_clientSecretEdit;
    QTextBrowser *m_instructionsText;
    QPushButton *m_helpButton;
};

#endif // CLOUDAUTHDIALOG_H
