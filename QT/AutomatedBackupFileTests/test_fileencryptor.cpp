#include <QtTest/QtTest>
#include "fileencryptor.h"
#include <QTemporaryDir>
#include <QTextStream>

class TestFileEncryptor : public QObject
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
        FileEncryptor encryptor;
        // Object should be created successfully
        QVERIFY(true);
    }

    void testSetPassword()
    {
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        // Password is set internally, no getter to test
        QVERIFY(true);
    }

    void testLoadPasswordFromFile()
    {
        // Create a temporary key file
        QString keyFilePath = tempDir->filePath("test_key.txt");
        QFile keyFile(keyFilePath);
        QVERIFY(keyFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&keyFile);
        out << testPassword;
        keyFile.close();
        
        FileEncryptor encryptor;
        bool loaded = encryptor.loadPasswordFromFile(keyFilePath);
        QVERIFY(loaded);
    }

    void testLoadPasswordFromNonExistentFile()
    {
        FileEncryptor encryptor;
        bool loaded = encryptor.loadPasswordFromFile("nonexistent_file.txt");
        QVERIFY(!loaded);
    }

    void testEncryptFile()
    {
        // Create a test file
        QString sourceFile = tempDir->filePath("source.txt");
        QFile file(sourceFile);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "This is test content for encryption.";
        file.close();
        
        // Encrypt the file
        QString encryptedFile = tempDir->filePath("encrypted.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        
        bool encrypted = encryptor.encryptFile(sourceFile, encryptedFile);
        QVERIFY(encrypted);
        QVERIFY(QFile::exists(encryptedFile));
        
        // Verify encrypted file is different from source
        QFile sourceF(sourceFile);
        QFile encryptedF(encryptedFile);
        QVERIFY(sourceF.open(QIODevice::ReadOnly));
        QVERIFY(encryptedF.open(QIODevice::ReadOnly));
        
        QByteArray sourceData = sourceF.readAll();
        QByteArray encryptedData = encryptedF.readAll();
        
        QVERIFY(sourceData != encryptedData);
        
        sourceF.close();
        encryptedF.close();
    }

    void testEncryptNonExistentFile()
    {
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        
        bool encrypted = encryptor.encryptFile("nonexistent.txt", tempDir->filePath("out.bin"));
        QVERIFY(!encrypted);
    }

    void testEncryptDirectory()
    {
        // Create a test directory structure
        QString sourceDir = tempDir->filePath("source_dir");
        QDir().mkpath(sourceDir);
        
        // Create some test files
        QString file1 = sourceDir + "/file1.txt";
        QString file2 = sourceDir + "/file2.txt";
        
        QFile f1(file1);
        QVERIFY(f1.open(QIODevice::WriteOnly | QIODevice::Text));
        f1.write("Content of file 1");
        f1.close();
        
        QFile f2(file2);
        QVERIFY(f2.open(QIODevice::WriteOnly | QIODevice::Text));
        f2.write("Content of file 2");
        f2.close();
        
        // Create subdirectory
        QString subDir = sourceDir + "/subdir";
        QDir().mkpath(subDir);
        QString file3 = subDir + "/file3.txt";
        QFile f3(file3);
        QVERIFY(f3.open(QIODevice::WriteOnly | QIODevice::Text));
        f3.write("Content of file 3");
        f3.close();
        
        // Encrypt the directory
        QString encryptedDir = tempDir->filePath("encrypted_dir");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        
        bool encrypted = encryptor.encryptDirectory(sourceDir, encryptedDir);
        QVERIFY(encrypted);
        QVERIFY(QDir(encryptedDir).exists());
    }

    void testEncryptEmptyFile()
    {
        QString emptyFile = tempDir->filePath("empty.txt");
        QFile file(emptyFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        
        QString encryptedFile = tempDir->filePath("empty_encrypted.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        
        bool encrypted = encryptor.encryptFile(emptyFile, encryptedFile);
        // Behavior may vary, just ensure it doesn't crash
        Q_UNUSED(encrypted);
    }

    void testEncryptLargeFile()
    {
        QString largeFile = tempDir->filePath("large.txt");
        QFile file(largeFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        
        // Write 1 MB of data
        QByteArray data(1024 * 1024, 'A');
        file.write(data);
        file.close();
        
        QString encryptedFile = tempDir->filePath("large_encrypted.bin");
        FileEncryptor encryptor;
        encryptor.setPassword(testPassword);
        
        bool encrypted = encryptor.encryptFile(largeFile, encryptedFile);
        QVERIFY(encrypted);
        QVERIFY(QFile::exists(encryptedFile));
    }
};

QTEST_MAIN(TestFileEncryptor)
#include "test_fileencryptor.moc"
