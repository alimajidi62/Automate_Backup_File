#ifndef NETWORKCREDENTIALSDIALOG_H
#define NETWORKCREDENTIALSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QString>

class NetworkCredentialsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkCredentialsDialog(const QString &networkPath, QWidget *parent = nullptr);
    ~NetworkCredentialsDialog();

    QString getUsername() const;
    QString getPassword() const;
    QString getDomain() const;
    bool shouldSaveCredentials() const;

private:
    void setupUI();

    QString m_networkPath;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_domainEdit;
    QCheckBox *m_saveCredentialsCheck;
};

#endif // NETWORKCREDENTIALSDIALOG_H
