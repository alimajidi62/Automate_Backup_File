#ifndef CLOUDPROVIDER_H
#define CLOUDPROVIDER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

// Base class for all cloud providers
class CloudProvider : public QObject
{
    Q_OBJECT

public:
    enum CloudProviderType {
        GoogleDrive,
        Dropbox,
        OneDrive,
        AmazonS3,
        BackblazeB2,
        Custom
    };

    enum ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    explicit CloudProvider(QObject *parent = nullptr);
    virtual ~CloudProvider();

    // Pure virtual methods - must be implemented by derived classes
    virtual bool authenticate(const QMap<QString, QString> &credentials) = 0;
    virtual bool testConnection() = 0;
    virtual bool uploadFile(const QString &localPath, const QString &remotePath) = 0;
    virtual bool downloadFile(const QString &remotePath, const QString &localPath) = 0;
    virtual bool deleteFile(const QString &remotePath) = 0;
    virtual bool listFiles(const QString &remotePath, QStringList &files) = 0;
    virtual qint64 getAvailableSpace() = 0;
    virtual qint64 getTotalSpace() = 0;
    virtual QString getProviderName() const = 0;
    virtual CloudProviderType getProviderType() const = 0;

    // Common functionality
    ConnectionStatus getStatus() const { return m_status; }
    QString getLastError() const { return m_lastError; }
    bool isAuthenticated() const { return m_authenticated; }

signals:
    void connectionStatusChanged(ConnectionStatus status);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void operationCompleted(bool success, const QString &message);
    void error(const QString &errorMessage);

protected:
    void setStatus(ConnectionStatus status);
    void setError(const QString &error);
    void setAuthenticated(bool authenticated);

    QNetworkAccessManager *m_networkManager;
    ConnectionStatus m_status;
    QString m_lastError;
    bool m_authenticated;
    QMap<QString, QString> m_credentials;
};

// Google Drive implementation
class GoogleDriveProvider : public CloudProvider
{
    Q_OBJECT

public:
    explicit GoogleDriveProvider(QObject *parent = nullptr);
    ~GoogleDriveProvider() override;

    bool authenticate(const QMap<QString, QString> &credentials) override;
    bool testConnection() override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool deleteFile(const QString &remotePath) override;
    bool listFiles(const QString &remotePath, QStringList &files) override;
    qint64 getAvailableSpace() override;
    qint64 getTotalSpace() override;
    QString getProviderName() const override { return "Google Drive"; }
    CloudProviderType getProviderType() const override { return GoogleDrive; }

private slots:
    void onAuthenticationFinished();
    void onRequestFinished(QNetworkReply *reply);

private:
    QString m_accessToken;
    QString m_refreshToken;
    QDateTime m_tokenExpiry;
    
    bool refreshAccessToken();
    QNetworkRequest createAuthenticatedRequest(const QString &url);
};

// Dropbox implementation
class DropboxProvider : public CloudProvider
{
    Q_OBJECT

public:
    explicit DropboxProvider(QObject *parent = nullptr);
    ~DropboxProvider() override;

    bool authenticate(const QMap<QString, QString> &credentials) override;
    bool testConnection() override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool deleteFile(const QString &remotePath) override;
    bool listFiles(const QString &remotePath, QStringList &files) override;
    qint64 getAvailableSpace() override;
    qint64 getTotalSpace() override;
    QString getProviderName() const override { return "Dropbox"; }
    CloudProviderType getProviderType() const override { return Dropbox; }

private slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QString m_accessToken;
    
    QNetworkRequest createAuthenticatedRequest(const QString &url);
};

// OneDrive implementation
class OneDriveProvider : public CloudProvider
{
    Q_OBJECT

public:
    explicit OneDriveProvider(QObject *parent = nullptr);
    ~OneDriveProvider() override;

    bool authenticate(const QMap<QString, QString> &credentials) override;
    bool testConnection() override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool deleteFile(const QString &remotePath) override;
    bool listFiles(const QString &remotePath, QStringList &files) override;
    qint64 getAvailableSpace() override;
    qint64 getTotalSpace() override;
    QString getProviderName() const override { return "OneDrive"; }
    CloudProviderType getProviderType() const override { return OneDrive; }

private slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QString m_accessToken;
    QString m_refreshToken;
    QDateTime m_tokenExpiry;
    
    bool refreshAccessToken();
    QNetworkRequest createAuthenticatedRequest(const QString &url);
};

// Amazon S3 implementation
class AmazonS3Provider : public CloudProvider
{
    Q_OBJECT

public:
    explicit AmazonS3Provider(QObject *parent = nullptr);
    ~AmazonS3Provider() override;

    bool authenticate(const QMap<QString, QString> &credentials) override;
    bool testConnection() override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool deleteFile(const QString &remotePath) override;
    bool listFiles(const QString &remotePath, QStringList &files) override;
    qint64 getAvailableSpace() override;
    qint64 getTotalSpace() override;
    QString getProviderName() const override { return "Amazon S3"; }
    CloudProviderType getProviderType() const override { return AmazonS3; }

private:
    QString m_accessKeyId;
    QString m_secretAccessKey;
    QString m_bucketName;
    QString m_region;
    
    QString generateAuthorizationHeader(const QString &method, const QString &resource);
    QNetworkRequest createAuthenticatedRequest(const QString &url, const QString &method);
};

// Mock Cloud Provider for Testing
class MockCloudProvider : public CloudProvider
{
    Q_OBJECT

public:
    explicit MockCloudProvider(QObject *parent = nullptr);
    ~MockCloudProvider() override;

    bool authenticate(const QMap<QString, QString> &credentials) override;
    bool testConnection() override;
    bool uploadFile(const QString &localPath, const QString &remotePath) override;
    bool downloadFile(const QString &remotePath, const QString &localPath) override;
    bool deleteFile(const QString &remotePath) override;
    bool listFiles(const QString &remotePath, QStringList &files) override;
    qint64 getAvailableSpace() override;
    qint64 getTotalSpace() override;
    QString getProviderName() const override { return "Mock Cloud (Test Mode)"; }
    CloudProviderType getProviderType() const override { return Custom; }

private:
    QStringList m_mockFiles;
    qint64 m_mockTotalSpace;
    qint64 m_mockUsedSpace;
};

// Factory for creating cloud providers
class CloudProviderFactory
{
public:
    static CloudProvider* createProvider(CloudProvider::CloudProviderType type, QObject *parent = nullptr);
    static CloudProvider* createProvider(const QString &providerName, QObject *parent = nullptr);
    static QStringList getAvailableProviders();
};

#endif // CLOUDPROVIDER_H
