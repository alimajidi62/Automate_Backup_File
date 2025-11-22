#include "networkcredentialsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>

NetworkCredentialsDialog::NetworkCredentialsDialog(const QString &networkPath, QWidget *parent)
    : QDialog(parent)
    , m_networkPath(networkPath)
{
    setWindowTitle(tr("Network Credentials Required"));
    setupUI();
}

NetworkCredentialsDialog::~NetworkCredentialsDialog()
{
}

void NetworkCredentialsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Info label
    QLabel *infoLabel = new QLabel(
        tr("The network path requires authentication:\n%1").arg(m_networkPath)
    );
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Credentials group
    QGroupBox *credentialsGroup = new QGroupBox(tr("Credentials"));
    QFormLayout *formLayout = new QFormLayout(credentialsGroup);
    
    // Username
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Enter username");
    formLayout->addRow(tr("Username:"), m_usernameEdit);
    
    // Password
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter password");
    formLayout->addRow(tr("Password:"), m_passwordEdit);
    
    // Domain (optional)
    m_domainEdit = new QLineEdit();
    m_domainEdit->setPlaceholderText("Optional (e.g., WORKGROUP)");
    formLayout->addRow(tr("Domain:"), m_domainEdit);
    
    mainLayout->addWidget(credentialsGroup);
    
    // Save credentials checkbox
    m_saveCredentialsCheck = new QCheckBox(tr("Remember credentials for this session"));
    m_saveCredentialsCheck->setChecked(true);
    mainLayout->addWidget(m_saveCredentialsCheck);
    
    // Warning label
    QLabel *warningLabel = new QLabel(
        tr("<i>Note: Credentials are stored in memory only during this session "
           "and are not saved to disk for security reasons.</i>")
    );
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(warningLabel);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    resize(450, 300);
    
    // Set focus to username field
    m_usernameEdit->setFocus();
}

QString NetworkCredentialsDialog::getUsername() const
{
    return m_usernameEdit->text();
}

QString NetworkCredentialsDialog::getPassword() const
{
    return m_passwordEdit->text();
}

QString NetworkCredentialsDialog::getDomain() const
{
    return m_domainEdit->text();
}

bool NetworkCredentialsDialog::shouldSaveCredentials() const
{
    return m_saveCredentialsCheck->isChecked();
}
