#ifndef FILEENCRYPTOR_H
#define FILEENCRYPTOR_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>

class FileEncryptor
{
public:
    FileEncryptor();
    
    // Load password from key.txt file
    bool loadPasswordFromFile(const QString& keyFilePath);
    
    // Set password directly
    void setPassword(const QString& password);
    
    // Encrypt a single file
    bool encryptFile(const QString& sourceFilePath, const QString& encryptedFilePath);
    
    // Encrypt entire directory recursively
    bool encryptDirectory(const QString& sourceDir, const QString& encryptedDir);
    
private:
    QString m_password;
    
    // XOR-based encryption with password
    QByteArray encryptData(const QByteArray& data);
    
    // Generate key from password
    QByteArray generateKey();
};

#endif // FILEENCRYPTOR_H
