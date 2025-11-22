#include "networkcredentialsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QFont>

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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title label
    QLabel *titleLabel = new QLabel(tr("Network Authentication Required"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Info frame
    QFrame *infoFrame = new QFrame();
    infoFrame->setObjectName("infoFrame");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    
    QLabel *infoLabel = new QLabel(
        tr("The network path requires authentication:\n<b>%1</b>").arg(m_networkPath)
    );
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    
    mainLayout->addWidget(infoFrame);
    
    // Credentials group
    QGroupBox *credentialsGroup = new QGroupBox(tr("Credentials"));
    QFormLayout *formLayout = new QFormLayout(credentialsGroup);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(15, 20, 15, 15);
    
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
    
    // Warning frame
    QFrame *warningFrame = new QFrame();
    warningFrame->setObjectName("warningFrame");
    QVBoxLayout *warningLayout = new QVBoxLayout(warningFrame);
    
    QLabel *warningLabel = new QLabel(
        tr("⚠️ Note: Credentials are stored in memory only during this session "
           "and are not saved to disk for security reasons.")
    );
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel);
    
    mainLayout->addWidget(warningFrame);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    resize(500, 400);
    
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
