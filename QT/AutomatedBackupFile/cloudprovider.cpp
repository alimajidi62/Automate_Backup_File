#include "cloudprovider.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QTimer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QThread>

// ============================================================================
// CloudProvider Base Class Implementation
// ============================================================================

CloudProvider::CloudProvider(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_status(Disconnected)
    , m_authenticated(false)
{
}

CloudProvider::~CloudProvider()
{
}

void CloudProvider::setStatus(ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit connectionStatusChanged(status);
    }
}

void CloudProvider::setError(const QString &error)
{
    m_lastError = error;
    emit this->error(error);
}

void CloudProvider::setAuthenticated(bool authenticated)
{
    m_authenticated = authenticated;
}

// ============================================================================
// Google Drive Provider Implementation
// ============================================================================

GoogleDriveProvider::GoogleDriveProvider(QObject *parent)
    : CloudProvider(parent)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &GoogleDriveProvider::onRequestFinished);
}

GoogleDriveProvider::~GoogleDriveProvider()
{
}

bool GoogleDriveProvider::authenticate(const QMap<QString, QString> &credentials)
{
    m_credentials = credentials;
    
    // Check if we have required credentials
    if (!credentials.contains("client_id") || !credentials.contains("client_secret")) {
        setError("Missing client_id or client_secret");
        return false;
    }
    
    // If we have an access token, try to use it
    if (credentials.contains("access_token")) {
        m_accessToken = credentials["access_token"];
        
        // Test the connection
        if (testConnection()) {
            setAuthenticated(true);
            setStatus(Connected);
            return true;
        }
    }
    
    // If we have a refresh token, try to refresh
    if (credentials.contains("refresh_token")) {
        m_refreshToken = credentials["refresh_token"];
        return refreshAccessToken();
    }
    
    setError("OAuth flow not implemented - please provide access_token");
    return false;
}

bool GoogleDriveProvider::testConnection()
{
    if (m_accessToken.isEmpty()) {
        setError("No access token available");
        return false;
    }
    
    setStatus(Connecting);
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://www.googleapis.com/drive/v3/about?fields=user,storageQuota"
    );
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    // Wait for response
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit); // 10 second timeout
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        setStatus(Connected);
        setAuthenticated(true);
        reply->deleteLater();
        return true;
    } else {
        setError("Connection test failed: " + reply->errorString());
        setStatus(Error);
        reply->deleteLater();
        return false;
    }
}

bool GoogleDriveProvider::uploadFile(const QString &localPath, const QString &remotePath)
{
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Cannot open file: " + localPath);
        return false;
    }
    
    QFileInfo fileInfo(localPath);
    
    // Create metadata
    QJsonObject metadata;
    metadata["name"] = fileInfo.fileName();
    if (!remotePath.isEmpty()) {
        // Note: remotePath should be the parent folder ID in Google Drive
        QJsonArray parents;
        parents.append(remotePath);
        metadata["parents"] = parents;
    }
    
    // Create multipart request
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
    
    QHttpPart metadataPart;
    metadataPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
    metadataPart.setBody(QJsonDocument(metadata).toJson());
    
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    filePart.setBodyDevice(&file);
    
    multiPart->append(metadataPart);
    multiPart->append(filePart);
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"
    );
    
    QNetworkReply *reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::uploadProgress, this, &CloudProvider::uploadProgress);
    
    // Wait for completion
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setError("Upload failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    file.close();
    
    return success;
}

bool GoogleDriveProvider::downloadFile(const QString &remotePath, const QString &localPath)
{
    // remotePath should be the file ID
    QNetworkRequest request = createAuthenticatedRequest(
        QString("https://www.googleapis.com/drive/v3/files/%1?alt=media").arg(remotePath)
    );
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, this, &CloudProvider::downloadProgress);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            reply->deleteLater();
            return true;
        } else {
            setError("Cannot write to file: " + localPath);
        }
    } else {
        setError("Download failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    return false;
}

bool GoogleDriveProvider::deleteFile(const QString &remotePath)
{
    QNetworkRequest request = createAuthenticatedRequest(
        QString("https://www.googleapis.com/drive/v3/files/%1").arg(remotePath)
    );
    
    QNetworkReply *reply = m_networkManager->deleteResource(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setError("Delete failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    return success;
}

bool GoogleDriveProvider::listFiles(const QString &remotePath, QStringList &files)
{
    QString url = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,mimeType)";
    if (!remotePath.isEmpty()) {
        url += QString("&q='%1'+in+parents").arg(remotePath);
    }
    
    QNetworkRequest request = createAuthenticatedRequest(url);
    QNetworkReply *reply = m_networkManager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray filesArray = doc.object()["files"].toArray();
        
        for (const QJsonValue &value : filesArray) {
            QJsonObject fileObj = value.toObject();
            files.append(fileObj["name"].toString());
        }
        
        reply->deleteLater();
        return true;
    }
    
    setError("List files failed: " + reply->errorString());
    reply->deleteLater();
    return false;
}

qint64 GoogleDriveProvider::getAvailableSpace()
{
    QNetworkRequest request = createAuthenticatedRequest(
        "https://www.googleapis.com/drive/v3/about?fields=storageQuota"
    );
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject quota = doc.object()["storageQuota"].toObject();
        
        qint64 limit = quota["limit"].toString().toLongLong();
        qint64 usage = quota["usage"].toString().toLongLong();
        
        reply->deleteLater();
        return limit - usage;
    }
    
    reply->deleteLater();
    return 0;
}

qint64 GoogleDriveProvider::getTotalSpace()
{
    QNetworkRequest request = createAuthenticatedRequest(
        "https://www.googleapis.com/drive/v3/about?fields=storageQuota"
    );
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject quota = doc.object()["storageQuota"].toObject();
        
        qint64 limit = quota["limit"].toString().toLongLong();
        reply->deleteLater();
        return limit;
    }
    
    reply->deleteLater();
    return 0;
}

bool GoogleDriveProvider::refreshAccessToken()
{
    // Implementation would make OAuth refresh token request
    setError("Token refresh not implemented");
    return false;
}

QNetworkRequest GoogleDriveProvider::createAuthenticatedRequest(const QString &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    return request;
}

void GoogleDriveProvider::onAuthenticationFinished()
{
    // Handle OAuth flow completion
}

void GoogleDriveProvider::onRequestFinished(QNetworkReply *reply)
{
    // Handle async request completion
    reply->deleteLater();
}

// ============================================================================
// Dropbox Provider Implementation
// ============================================================================

DropboxProvider::DropboxProvider(QObject *parent)
    : CloudProvider(parent)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &DropboxProvider::onRequestFinished);
}

DropboxProvider::~DropboxProvider()
{
}

bool DropboxProvider::authenticate(const QMap<QString, QString> &credentials)
{
    m_credentials = credentials;
    
    if (!credentials.contains("access_token")) {
        setError("Missing access_token for Dropbox");
        return false;
    }
    
    m_accessToken = credentials["access_token"];
    
    if (testConnection()) {
        setAuthenticated(true);
        setStatus(Connected);
        return true;
    }
    
    return false;
}

bool DropboxProvider::testConnection()
{
    QNetworkRequest request = createAuthenticatedRequest(
        "https://api.dropboxapi.com/2/users/get_current_account"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, "null");
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (success) {
        setStatus(Connected);
        setAuthenticated(true);
    } else {
        setError("Connection test failed: " + reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
    return success;
}

bool DropboxProvider::uploadFile(const QString &localPath, const QString &remotePath)
{
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Cannot open file: " + localPath);
        return false;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    QJsonObject args;
    args["path"] = remotePath;
    args["mode"] = "add";
    args["autorename"] = true;
    args["mute"] = false;
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://content.dropboxapi.com/2/files/upload"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Dropbox-API-Arg", QJsonDocument(args).toJson(QJsonDocument::Compact));
    
    QNetworkReply *reply = m_networkManager->post(request, fileData);
    connect(reply, &QNetworkReply::uploadProgress, this, &CloudProvider::uploadProgress);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setError("Upload failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    return success;
}

bool DropboxProvider::downloadFile(const QString &remotePath, const QString &localPath)
{
    QJsonObject args;
    args["path"] = remotePath;
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://content.dropboxapi.com/2/files/download"
    );
    request.setRawHeader("Dropbox-API-Arg", QJsonDocument(args).toJson(QJsonDocument::Compact));
    
    QNetworkReply *reply = m_networkManager->post(request, QByteArray());
    connect(reply, &QNetworkReply::downloadProgress, this, &CloudProvider::downloadProgress);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            reply->deleteLater();
            return true;
        } else {
            setError("Cannot write to file: " + localPath);
        }
    } else {
        setError("Download failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    return false;
}

bool DropboxProvider::deleteFile(const QString &remotePath)
{
    QJsonObject args;
    args["path"] = remotePath;
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://api.dropboxapi.com/2/files/delete_v2"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, 
        QJsonDocument(args).toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setError("Delete failed: " + reply->errorString());
    }
    
    reply->deleteLater();
    return success;
}

bool DropboxProvider::listFiles(const QString &remotePath, QStringList &files)
{
    QJsonObject args;
    args["path"] = remotePath.isEmpty() ? "" : remotePath;
    args["recursive"] = false;
    
    QNetworkRequest request = createAuthenticatedRequest(
        "https://api.dropboxapi.com/2/files/list_folder"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request,
        QJsonDocument(args).toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray entries = doc.object()["entries"].toArray();
        
        for (const QJsonValue &value : entries) {
            QJsonObject entry = value.toObject();
            files.append(entry["name"].toString());
        }
        
        reply->deleteLater();
        return true;
    }
    
    setError("List files failed: " + reply->errorString());
    reply->deleteLater();
    return false;
}

qint64 DropboxProvider::getAvailableSpace()
{
    QNetworkRequest request = createAuthenticatedRequest(
        "https://api.dropboxapi.com/2/users/get_space_usage"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, "null");
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        
        qint64 allocated = obj["allocation"].toObject()["allocated"].toVariant().toLongLong();
        qint64 used = obj["used"].toVariant().toLongLong();
        
        reply->deleteLater();
        return allocated - used;
    }
    
    reply->deleteLater();
    return 0;
}

qint64 DropboxProvider::getTotalSpace()
{
    QNetworkRequest request = createAuthenticatedRequest(
        "https://api.dropboxapi.com/2/users/get_space_usage"
    );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = m_networkManager->post(request, "null");
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        qint64 allocated = doc.object()["allocation"].toObject()["allocated"].toVariant().toLongLong();
        reply->deleteLater();
        return allocated;
    }
    
    reply->deleteLater();
    return 0;
}

QNetworkRequest DropboxProvider::createAuthenticatedRequest(const QString &url)
{
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    return request;
}

void DropboxProvider::onRequestFinished(QNetworkReply *reply)
{
    reply->deleteLater();
}

// ============================================================================
// OneDrive Provider Implementation (Placeholder)
// ============================================================================

OneDriveProvider::OneDriveProvider(QObject *parent)
    : CloudProvider(parent)
{
}

OneDriveProvider::~OneDriveProvider()
{
}

bool OneDriveProvider::authenticate(const QMap<QString, QString> &credentials)
{
    setError("OneDrive provider not fully implemented yet");
    return false;
}

bool OneDriveProvider::testConnection()
{
    return false;
}

bool OneDriveProvider::uploadFile(const QString &, const QString &)
{
    setError("OneDrive upload not implemented");
    return false;
}

bool OneDriveProvider::downloadFile(const QString &, const QString &)
{
    setError("OneDrive download not implemented");
    return false;
}

bool OneDriveProvider::deleteFile(const QString &)
{
    setError("OneDrive delete not implemented");
    return false;
}

bool OneDriveProvider::listFiles(const QString &, QStringList &)
{
    setError("OneDrive list not implemented");
    return false;
}

qint64 OneDriveProvider::getAvailableSpace()
{
    return 0;
}

qint64 OneDriveProvider::getTotalSpace()
{
    return 0;
}

bool OneDriveProvider::refreshAccessToken()
{
    return false;
}

QNetworkRequest OneDriveProvider::createAuthenticatedRequest(const QString &url)
{
    return QNetworkRequest(url);
}

void OneDriveProvider::onRequestFinished(QNetworkReply *reply)
{
    reply->deleteLater();
}

// ============================================================================
// Amazon S3 Provider Implementation (Placeholder)
// ============================================================================

AmazonS3Provider::AmazonS3Provider(QObject *parent)
    : CloudProvider(parent)
{
}

AmazonS3Provider::~AmazonS3Provider()
{
}

bool AmazonS3Provider::authenticate(const QMap<QString, QString> &credentials)
{
    setError("Amazon S3 provider not fully implemented yet");
    return false;
}

bool AmazonS3Provider::testConnection()
{
    return false;
}

bool AmazonS3Provider::uploadFile(const QString &, const QString &)
{
    setError("S3 upload not implemented");
    return false;
}

bool AmazonS3Provider::downloadFile(const QString &, const QString &)
{
    setError("S3 download not implemented");
    return false;
}

bool AmazonS3Provider::deleteFile(const QString &)
{
    setError("S3 delete not implemented");
    return false;
}

bool AmazonS3Provider::listFiles(const QString &, QStringList &)
{
    setError("S3 list not implemented");
    return false;
}

qint64 AmazonS3Provider::getAvailableSpace()
{
    return 0;
}

qint64 AmazonS3Provider::getTotalSpace()
{
    return 0;
}

QString AmazonS3Provider::generateAuthorizationHeader(const QString &, const QString &)
{
    return QString();
}

QNetworkRequest AmazonS3Provider::createAuthenticatedRequest(const QString &url, const QString &)
{
    return QNetworkRequest(url);
}

// ============================================================================
// Mock Cloud Provider Implementation (for testing)
// ============================================================================

MockCloudProvider::MockCloudProvider(QObject *parent)
    : CloudProvider(parent)
    , m_mockTotalSpace(100LL * 1024 * 1024 * 1024) // 100 GB
    , m_mockUsedSpace(25LL * 1024 * 1024 * 1024)  // 25 GB used
{
}

MockCloudProvider::~MockCloudProvider()
{
}

bool MockCloudProvider::authenticate(const QMap<QString, QString> &credentials)
{
    m_credentials = credentials;
    setAuthenticated(true);
    setStatus(Connected);
    return true;
}

bool MockCloudProvider::testConnection()
{
    setStatus(Connecting);
    // Simulate network delay
    QThread::msleep(500);
    setStatus(Connected);
    setAuthenticated(true);
    return true;
}

bool MockCloudProvider::uploadFile(const QString &localPath, const QString &remotePath)
{
    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists()) {
        setError("Local file does not exist");
        return false;
    }
    
    qint64 fileSize = fileInfo.size();
    
    // Simulate upload progress
    for (int i = 0; i <= 100; i += 10) {
        emit uploadProgress(fileSize * i / 100, fileSize);
        QThread::msleep(50);
    }
    
    m_mockFiles.append(remotePath);
    m_mockUsedSpace += fileSize;
    
    emit operationCompleted(true, "File uploaded successfully");
    return true;
}

bool MockCloudProvider::downloadFile(const QString &remotePath, const QString &localPath)
{
    if (!m_mockFiles.contains(remotePath)) {
        setError("Remote file does not exist");
        return false;
    }
    
    qint64 fakeSize = 1024 * 1024; // 1 MB
    
    // Simulate download progress
    for (int i = 0; i <= 100; i += 10) {
        emit downloadProgress(fakeSize * i / 100, fakeSize);
        QThread::msleep(50);
    }
    
    // Create a dummy file
    QFile file(localPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("Mock cloud file content");
        file.close();
        emit operationCompleted(true, "File downloaded successfully");
        return true;
    }
    
    setError("Cannot write to local file");
    return false;
}

bool MockCloudProvider::deleteFile(const QString &remotePath)
{
    if (m_mockFiles.removeOne(remotePath)) {
        emit operationCompleted(true, "File deleted successfully");
        return true;
    }
    
    setError("File not found");
    return false;
}

bool MockCloudProvider::listFiles(const QString &remotePath, QStringList &files)
{
    Q_UNUSED(remotePath);
    files = m_mockFiles;
    return true;
}

qint64 MockCloudProvider::getAvailableSpace()
{
    return m_mockTotalSpace - m_mockUsedSpace;
}

qint64 MockCloudProvider::getTotalSpace()
{
    return m_mockTotalSpace;
}

// ============================================================================
// Cloud Provider Factory Implementation
// ============================================================================

CloudProvider* CloudProviderFactory::createProvider(CloudProvider::CloudProviderType type, QObject *parent)
{
    switch (type) {
        case CloudProvider::GoogleDrive:
            return new GoogleDriveProvider(parent);
        case CloudProvider::Dropbox:
            return new DropboxProvider(parent);
        case CloudProvider::OneDrive:
            return new OneDriveProvider(parent);
        case CloudProvider::AmazonS3:
            return new AmazonS3Provider(parent);
        default:
            return nullptr;
    }
}

CloudProvider* CloudProviderFactory::createProvider(const QString &providerName, QObject *parent)
{
    if (providerName == "Google Drive") {
        return new GoogleDriveProvider(parent);
    } else if (providerName == "Dropbox") {
        return new DropboxProvider(parent);
    } else if (providerName == "OneDrive") {
        return new OneDriveProvider(parent);
    } else if (providerName == "Amazon S3") {
        return new AmazonS3Provider(parent);
    }
    
    return nullptr;
}

QStringList CloudProviderFactory::getAvailableProviders()
{
    return QStringList() << "Google Drive" << "Dropbox" << "OneDrive" << "Amazon S3";
}
