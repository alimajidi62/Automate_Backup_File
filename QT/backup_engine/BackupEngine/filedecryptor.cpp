#include "filedecryptor.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

FileDecryptor::FileDecryptor()
{
}

bool FileDecryptor::loadPasswordFromFile(const QString& keyFilePath)
{
    QFile keyFile(keyFilePath);
    if (!keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open key file:" << keyFilePath;
        return false;
    }
    
    m_password = keyFile.readAll().trimmed();
    keyFile.close();
    
    if (m_password.isEmpty()) {
        qWarning() << "Password is empty in key file";
        return false;
    }
    
    qDebug() << "Password loaded successfully from" << keyFilePath;
    return true;
}

void FileDecryptor::setPassword(const QString& password)
{
    m_password = password;
}

QByteArray FileDecryptor::generateKey()
{
    // Generate a hash-based key from password (same as encryption)
    QByteArray passwordBytes = m_password.toUtf8();
    QByteArray key = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return key;
}

QByteArray FileDecryptor::decryptData(const QByteArray& data)
{
    QByteArray key = generateKey();
    QByteArray decrypted;
    decrypted.resize(data.size());
    
    // XOR decryption (same as encryption)
    for (int i = 0; i < data.size(); ++i) {
        decrypted[i] = data[i] ^ key[i % key.size()];
    }
    
    return decrypted;
}

bool FileDecryptor::decryptFile(const QString& encryptedFilePath, const QString& decryptedFilePath)
{
    QFile encryptedFile(encryptedFilePath);
    if (!encryptedFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open encrypted file:" << encryptedFilePath;
        return false;
    }
    
    QByteArray encryptedData = encryptedFile.readAll();
    encryptedFile.close();
    
    QByteArray decryptedData = decryptData(encryptedData);
    
    // Create destination directory if needed
    QFileInfo fileInfo(decryptedFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile decryptedFile(decryptedFilePath);
    if (!decryptedFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create decrypted file:" << decryptedFilePath;
        return false;
    }
    
    decryptedFile.write(decryptedData);
    decryptedFile.close();
    
    qDebug() << "Decrypted:" << encryptedFilePath << "->" << decryptedFilePath;
    return true;
}

bool FileDecryptor::decryptDirectory(const QString& encryptedBackupDir)
{
    QDir encryptedDir(encryptedBackupDir);
    if (!encryptedDir.exists()) {
        qWarning() << "Encrypted backup directory does not exist:" << encryptedBackupDir;
        return false;
    }
    
    // Create decrypted subfolder inside backup directory
    QString decryptedDir = encryptedBackupDir + "/decrypted";
    QDir().mkpath(decryptedDir);
    
    qDebug() << "Decrypting files to:" << decryptedDir;
    
    QDirIterator it(encryptedBackupDir, QStringList() << "*.enc", QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    bool allSuccess = true;
    while (it.hasNext()) {
        QString encryptedFile = it.next();
        
        // Skip files in the decrypted folder itself
        if (encryptedFile.contains("/decrypted/")) {
            continue;
        }
        
        QString relativePath = encryptedDir.relativeFilePath(encryptedFile);
        
        // Remove .enc extension
        if (relativePath.endsWith(".enc")) {
            relativePath.chop(4);
        }
        
        QString decryptedFile = decryptedDir + "/" + relativePath;
        
        if (!decryptFile(encryptedFile, decryptedFile)) {
            allSuccess = false;
        }
    }
    
    if (allSuccess) {
        qDebug() << "All files decrypted successfully to:" << decryptedDir;
    }
    
    return allSuccess;
}
