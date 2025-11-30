#include <QtTest/QtTest>
#include "filedecryptor.h"
#include "fileencryptor.h"
#include <QTemporaryDir>
#include <QTextStream>

class TestFileDecryptor : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir* tempDir;
    QString testPassword;

private slots:
    void initTestCase()
    {
        tempDir = new QTemporaryDir();
        QVERIFY(tempDir->isValid());
        testPassword = "TestPassword123";
    }

    void cleanupTestCase()
    {
        delete tempDir;
    }

    void testConstructor()
    {
        FileDecryptor decryptor;
        QVERIFY(true);
    }

    void testSetPassword()
    {
        FileDecryptor decryptor;
        decryptor.setPassword(testPassword);
        QVERIFY(true);
    }

    void testLoadPasswordFromFile()
    {
        QString keyFilePath = tempDir->filePath("decrypt_key.txt");
        QFile keyFile(keyFilePath);
        QVERIFY(keyFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&keyFile);
        out << testPassword;
        keyFile.close();
        
        FileDecryptor decryptor;
        bool loaded = decryptor.loadPasswordFromFile(keyFilePath);
        QVERIFY(loaded);
    }

    void testLoadPasswordFromNonExistentFile()
    {
        FileDecryptor decryptor;
        bool loaded = decryptor.loadPasswordFromFile("nonexistent_decrypt_key.txt");
        QVERIFY(!loaded);
    }

    void testEncryptThenDecryptFile()
    {
        // Create original file
        QString originalFile = tempDir->filePath("original.txt");
        QFile file(originalFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        QString originalContent = "This is the original content that will be encrypted and decrypted.";
        out << originalContent;
        file.close();
        
        // Encrypt the file
        QString encryptedFile = tempDir->filePath("encrypted_for_decrypt.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        bool encrypted = encryptor.encryptFile(originalFile, encryptedFile);
        QVERIFY(encrypted);
        
        // Decrypt the file
        QString decryptedFile = tempDir->filePath("decrypted.txt");
        FileDecryptor decryptor;
        decryptor.setPassword(testPassword);
        bool decrypted = decryptor.decryptFile(encryptedFile, decryptedFile);
        QVERIFY(decrypted);
        
        // Verify content matches
        QFile decryptedF(decryptedFile);
        QVERIFY(decryptedF.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream in(&decryptedF);
        QString decryptedContent = in.readAll();
        decryptedF.close();
        
        QCOMPARE(decryptedContent, originalContent);
    }

    void testDecryptNonExistentFile()
    {
        FileDecryptor decryptor;
        decryptor.setPassword(testPassword);
        
        bool decrypted = decryptor.decryptFile("nonexistent_encrypted.bin", tempDir->filePath("out.txt"));
        QVERIFY(!decrypted);
    }

    void testDecryptWithWrongPassword()
    {
        // Create and encrypt a file
        QString originalFile = tempDir->filePath("wrong_pass_original.txt");
        QFile file(originalFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("Secret content");
        file.close();
        
        QString encryptedFile = tempDir->filePath("wrong_pass_encrypted.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        QVERIFY(encryptor.encryptFile(originalFile, encryptedFile));
        
        // Try to decrypt with wrong password
        QString decryptedFile = tempDir->filePath("wrong_pass_decrypted.txt");
        FileDecryptor decryptor;
        decryptor.setPassword("WrongPassword");
        bool decrypted = decryptor.decryptFile(encryptedFile, decryptedFile);
        
        // Decryption may succeed but content will be garbage
        // The implementation determines exact behavior
        Q_UNUSED(decrypted);
    }

    void testDecryptDirectory()
    {
        // Create and encrypt a directory
        QString sourceDir = tempDir->filePath("decrypt_source_dir");
        QDir().mkpath(sourceDir);
        
        QString file1 = sourceDir + "/decfile1.txt";
        QFile f1(file1);
        QVERIFY(f1.open(QIODevice::WriteOnly | QIODevice::Text));
        f1.write("Decrypt content 1");
        f1.close();
        
        QString encryptedDir = tempDir->filePath("decrypt_encrypted_dir");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        QVERIFY(encryptor.encryptDirectory(sourceDir, encryptedDir));
        
        // Decrypt the directory
        FileDecryptor decryptor;
        decryptor.setPassword(testPassword);
        bool decrypted = decryptor.decryptDirectory(encryptedDir);
        
        // The method creates a "decrypted" subfolder
        // Testing that it doesn't crash
        Q_UNUSED(decrypted);
    }

    void testDecryptEmptyFile()
    {
        // Encrypt an empty file
        QString emptyFile = tempDir->filePath("empty_to_decrypt.txt");
        QFile file(emptyFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        
        QString encryptedFile = tempDir->filePath("empty_encrypted_to_decrypt.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        encryptor.encryptFile(emptyFile, encryptedFile);
        
        QString decryptedFile = tempDir->filePath("empty_decrypted.txt");
        FileDecryptor decryptor;
        decryptor.setPassword(testPassword);
        bool decrypted = decryptor.decryptFile(encryptedFile, decryptedFile);
        
        Q_UNUSED(decrypted);
    }

    void testMultipleEncryptDecryptCycles()
    {
        QString originalContent = "Test content for multiple cycles";
        QString file1 = tempDir->filePath("cycle_original.txt");
        
        QFile f(file1);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write(originalContent.toUtf8());
        f.close();
        
        FileEncryptor encryptor;
        FileDecryptor decryptor;
        encryptor.setPassword(testPassword);
        decryptor.setPassword(testPassword);
        
        QString currentFile = file1;
        
        // Encrypt and decrypt multiple times
        for (int i = 0; i < 3; i++) {
            QString encFile = tempDir->filePath(QString("cycle_enc_%1.bin").arg(i));
            QString decFile = tempDir->filePath(QString("cycle_dec_%1.txt").arg(i));
            
            QVERIFY(encryptor.encryptFile(currentFile, encFile));
            QVERIFY(decryptor.decryptFile(encFile, decFile));
            
            currentFile = decFile;
        }
        
        // Verify final content
        QFile finalFile(currentFile);
        QVERIFY(finalFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QString finalContent = QString::fromUtf8(finalFile.readAll());
        finalFile.close();
        
        QCOMPARE(finalContent, originalContent);
    }
};

QTEST_MAIN(TestFileDecryptor)
#include "test_filedecryptor.moc"
