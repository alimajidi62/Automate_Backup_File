#include "cloudauthdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QDesktopServices>
#include <QUrl>

CloudAuthDialog::CloudAuthDialog(const QString &providerName, QWidget *parent)
    : QDialog(parent)
    , m_providerName(providerName)
    , m_testMode(false)
    , m_accessTokenEdit(nullptr)
    , m_clientIdEdit(nullptr)
    , m_clientSecretEdit(nullptr)
{
    setWindowTitle(tr("Cloud Authentication - %1").arg(providerName));
    setupUI();
}

CloudAuthDialog::~CloudAuthDialog()
{
}

void CloudAuthDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Test mode checkbox
    QCheckBox *testModeCheck = new QCheckBox(tr("Enable Test Mode (Mock Cloud Provider)"));
    testModeCheck->setToolTip(tr("Use a simulated cloud provider for testing without real credentials"));
    connect(testModeCheck, &QCheckBox::stateChanged, this, &CloudAuthDialog::onTestModeChanged);
    mainLayout->addWidget(testModeCheck);
    
    // Instructions
    m_instructionsText = new QTextBrowser();
    m_instructionsText->setMaximumHeight(150);
    m_instructionsText->setOpenExternalLinks(true);
    mainLayout->addWidget(m_instructionsText);
    
    // Create form layout for credentials
    QFormLayout *formLayout = new QFormLayout();
    
    // Provider-specific UI
    if (m_providerName == "Google Drive") {
        createGoogleDriveUI(formLayout);
    } else if (m_providerName == "Dropbox") {
        createDropboxUI(formLayout);
    } else {
        createGenericUI(formLayout);
    }
    
    mainLayout->addLayout(formLayout);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    resize(600, 500);
}

void CloudAuthDialog::createGoogleDriveUI(QFormLayout *formLayout)
{
    m_instructionsText->setHtml(
        tr("<h3>Google Drive Authentication</h3>"
           "<p>To use Google Drive, you need to:</p>"
           "<ol>"
           "<li>Go to <a href='https://console.cloud.google.com/'>Google Cloud Console</a></li>"
           "<li>Create a new project or select an existing one</li>"
           "<li>Enable the Google Drive API</li>"
           "<li>Create OAuth 2.0 credentials (Desktop application)</li>"
           "<li>Use the OAuth 2.0 Playground to get an access token:</li>"
           "<ul>"
           "<li>Visit <a href='https://developers.google.com/oauthplayground/'>OAuth 2.0 Playground</a></li>"
           "<li>Click settings (gear icon), check 'Use your own OAuth credentials'</li>"
           "<li>Enter your Client ID and Client Secret</li>"
           "<li>In Step 1, select 'Drive API v3' and authorize</li>"
           "<li>In Step 2, exchange authorization code for tokens</li>"
           "<li>Copy the 'Access token' value below</li>"
           "</ul>"
           "</ol>"
           "<p><b>OR</b> enable Test Mode above to simulate cloud storage without real credentials.</p>")
    );
    
    m_clientIdEdit = new QLineEdit();
    m_clientIdEdit->setPlaceholderText("Your OAuth Client ID (optional)");
    formLayout->addRow(tr("Client ID:"), m_clientIdEdit);
    
    m_clientSecretEdit = new QLineEdit();
    m_clientSecretEdit->setPlaceholderText("Your OAuth Client Secret (optional)");
    m_clientSecretEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("Client Secret:"), m_clientSecretEdit);
    
    m_accessTokenEdit = new QLineEdit();
    m_accessTokenEdit->setPlaceholderText("Paste access token here");
    formLayout->addRow(tr("Access Token:"), m_accessTokenEdit);
}

void CloudAuthDialog::createDropboxUI(QFormLayout *formLayout)
{
    m_instructionsText->setHtml(
        tr("<h3>Dropbox Authentication</h3>"
           "<p>To use Dropbox, you need to:</p>"
           "<ol>"
           "<li>Go to <a href='https://www.dropbox.com/developers/apps'>Dropbox App Console</a></li>"
           "<li>Create a new app (select 'Scoped access' and 'Full Dropbox')</li>"
           "<li>Go to the 'Permissions' tab and enable required scopes:</li>"
           "<ul>"
           "<li>files.metadata.write</li>"
           "<li>files.metadata.read</li>"
           "<li>files.content.write</li>"
           "<li>files.content.read</li>"
           "</ul>"
           "<li>Go to the 'Settings' tab</li>"
           "<li>Under 'OAuth 2', generate an access token</li>"
           "<li>Copy the access token and paste it below</li>"
           "</ol>"
           "<p><b>OR</b> enable Test Mode above to simulate cloud storage without real credentials.</p>")
    );
    
    m_accessTokenEdit = new QLineEdit();
    m_accessTokenEdit->setPlaceholderText("Paste access token here");
    formLayout->addRow(tr("Access Token:"), m_accessTokenEdit);
}

void CloudAuthDialog::createGenericUI(QFormLayout *formLayout)
{
    m_instructionsText->setHtml(
        tr("<h3>Cloud Authentication</h3>"
           "<p>This cloud provider requires authentication credentials.</p>"
           "<p>Please refer to your cloud provider's documentation for how to obtain an API key or access token.</p>"
           "<p><b>OR</b> enable Test Mode above to simulate cloud storage without real credentials.</p>")
    );
    
    m_accessTokenEdit = new QLineEdit();
    m_accessTokenEdit->setPlaceholderText("API Key / Access Token");
    formLayout->addRow(tr("Credentials:"), m_accessTokenEdit);
}

void CloudAuthDialog::onTestModeChanged(int state)
{
    m_testMode = (state == Qt::Checked);
    
    // Disable credential fields in test mode
    if (m_accessTokenEdit) {
        m_accessTokenEdit->setEnabled(!m_testMode);
        if (m_testMode) {
            m_accessTokenEdit->setPlaceholderText("Test mode - no credentials needed");
        }
    }
    if (m_clientIdEdit) {
        m_clientIdEdit->setEnabled(!m_testMode);
    }
    if (m_clientSecretEdit) {
        m_clientSecretEdit->setEnabled(!m_testMode);
    }
}

void CloudAuthDialog::onOpenInstructionsLink()
{
    // This slot is not used in current implementation
    // Instructions are shown as HTML with clickable links
}

QMap<QString, QString> CloudAuthDialog::getCredentials() const
{
    QMap<QString, QString> creds;
    
    if (m_testMode) {
        creds["test_mode"] = "true";
        return creds;
    }
    
    if (m_accessTokenEdit) {
        creds["access_token"] = m_accessTokenEdit->text();
    }
    if (m_clientIdEdit) {
        creds["client_id"] = m_clientIdEdit->text();
    }
    if (m_clientSecretEdit) {
        creds["client_secret"] = m_clientSecretEdit->text();
    }
    
    return creds;
}
