#include "fileencryptor.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

FileEncryptor::FileEncryptor()
{
}

bool FileEncryptor::loadPasswordFromFile(const QString& keyFilePath)
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

void FileEncryptor::setPassword(const QString& password)
{
    m_password = password;
}

QByteArray FileEncryptor::generateKey()
{
    // Generate a hash-based key from password
    QByteArray passwordBytes = m_password.toUtf8();
    QByteArray key = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return key;
}

QByteArray FileEncryptor::encryptData(const QByteArray& data)
{
    QByteArray key = generateKey();
    QByteArray encrypted;
    encrypted.resize(data.size());
    
    // XOR encryption with repeating key
    for (int i = 0; i < data.size(); ++i) {
        encrypted[i] = data[i] ^ key[i % key.size()];
    }
    
    return encrypted;
}

bool FileEncryptor::encryptFile(const QString& sourceFilePath, const QString& encryptedFilePath)
{
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open source file:" << sourceFilePath;
        return false;
    }
    
    QByteArray data = sourceFile.readAll();
    sourceFile.close();
    
    QByteArray encryptedData = encryptData(data);
    
    // Create destination directory if needed
    QFileInfo fileInfo(encryptedFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile encryptedFile(encryptedFilePath);
    if (!encryptedFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create encrypted file:" << encryptedFilePath;
        return false;
    }
    
    encryptedFile.write(encryptedData);
    encryptedFile.close();
    
    qDebug() << "Encrypted:" << sourceFilePath << "->" << encryptedFilePath;
    return true;
}

bool FileEncryptor::encryptDirectory(const QString& sourceDir, const QString& encryptedDir)
{
    QDir source(sourceDir);
    if (!source.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDir;
        return false;
    }
    
    QDir encrypted(encryptedDir);
    if (!encrypted.exists()) {
        encrypted.mkpath(".");
    }
    
    QDirIterator it(sourceDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    bool allSuccess = true;
    while (it.hasNext()) {
        QString sourceFile = it.next();
        QString relativePath = source.relativeFilePath(sourceFile);
        QString encryptedFile = encryptedDir + "/" + relativePath + ".enc";
        
        if (!encryptFile(sourceFile, encryptedFile)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}
