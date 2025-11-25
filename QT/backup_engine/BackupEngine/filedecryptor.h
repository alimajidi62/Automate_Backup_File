#ifndef FILEDECRYPTOR_H
#define FILEDECRYPTOR_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>

class FileDecryptor
{
public:
    FileDecryptor();
    
    // Load password from key.txt file
    bool loadPasswordFromFile(const QString& keyFilePath);
    
    // Set password directly
    void setPassword(const QString& password);
    
    // Decrypt a single file
    bool decryptFile(const QString& encryptedFilePath, const QString& decryptedFilePath);
    
    // Decrypt entire directory and save to "decrypted" subfolder
    // Creates: destinationBackupFolder/decrypted/...
    bool decryptDirectory(const QString& encryptedBackupDir);
    
private:
    QString m_password;
    
    // XOR-based decryption with password (same as encryption)
    QByteArray decryptData(const QByteArray& data);
    
    // Generate key from password
    QByteArray generateKey();
};

#endif // FILEDECRYPTOR_H
