// Rory Madden 2023

// compile: g++ -o CreateBackup CreateBackup.cpp -lcrypto
// from CowabungaLiteWindows: https://github.com/Avangelista/CowabungaLiteWindows/blob/main/CreateBackup.cpp

#include "CreateBackup.h"
#include <QDir>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <QString>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/installation_proxy.h>

QString removeDomain(const QString &domain, const QString &input)
{
    auto d = domain.toStdString();
    auto i = input.toStdString();

    size_t pos = i.find(d);
    if (pos != std::string::npos)
    {
        pos += d.length();
        if (i[pos] == '\\' || i[pos] == '/')
        {
            ++pos;
        }
        return QString::fromStdString(i.substr(pos));
    }
    return QString::fromStdString(i);
}

void writeStringWithLength(QFile &output_file, const QString &qstr)
{
    QByteArray byteArray = qstr.toUtf8();
    quint16 length = static_cast<quint16>(byteArray.size());

    quint16 bigEndianLength = (length << 8) | (length >> 8);

    output_file.write(reinterpret_cast<const char *>(&bigEndianLength), sizeof(bigEndianLength));
    output_file.write(byteArray);
}

void writeHash(QFile &output_file, const QString &file)
{
    QFile input_file(file);
    if (!input_file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file:" << file;
        return;
    }

    QByteArray fileContent = input_file.readAll();
    QByteArray hash = QCryptographicHash::hash(fileContent, QCryptographicHash::Sha1);
    output_file.write(hash);
}

void generateRandomHex(QFile &output_file)
{
    QRandomGenerator generator;
    generator.seed(QDateTime::currentMSecsSinceEpoch());

    constexpr int bufferSize = 12;
    QByteArray buffer;
    buffer.resize(bufferSize);

    for (int i = 0; i < bufferSize; ++i)
    {
        buffer[i] = static_cast<char>(generator.generate() % 256);
    }

    output_file.write(buffer);

    // For testing purposes
    // QByteArray testData(12, '\xFF');
    // output_file.write(testData);
}

std::string calculateSHA1(const std::string &str)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(str.c_str()), str.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

void copyFile(const QString &source, const QString &destination)
{
    QFile sourceFile(source);
    if (!sourceFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open source file:" << source;
        return;
    }

    QFile destinationFile(destination);
    if (!destinationFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to create destination file:" << destination;
        return;
    }

    destinationFile.write(sourceFile.readAll());
}

void processFiles(const QString &path, const QString &domainString, const QString &outputDir)
{
    QString fileString = removeDomain(domainString, path)
                            .replace("hiddendot", ".")
                            .replace("\\", "/");
    
    QFile output_file(outputDir + "/Manifest.mbdb");
    if (!output_file.open(QIODevice::Append))
    {
        qDebug() << "Failed to open output file";
        return;
    }

    if (domainString == "ConfigProfileDomain")
    {
        writeStringWithLength(output_file, "SysSharedContainerDomain-systemgroup.com.apple.configurationprofiles");
    }
    else if (domainString == "App-PB") {
        writeStringWithLength(output_file, "AppDomain-com.apple.PosterBoard");
    }
    else
    {
        writeStringWithLength(output_file, domainString);
    }
    writeStringWithLength(output_file, fileString);

    if (QFileInfo(path).isFile())
    {
        output_file.write("\xFF\xFF\x00\x14", 4);
        writeHash(output_file, path);
        output_file.write("\xFF\xFF\x81\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xF5\x00\x00\x01\xF5", 20);
        generateRandomHex(output_file);
        QFileInfo fileInfo(path);
        qint64 fileSize = fileInfo.size();
        QByteArray sizeBytes;

        // Convert to big endian
        for (int i = 7; i >= 0; --i)
        {
            unsigned char byte = static_cast<unsigned char>((fileSize >> (8 * i)) & 0xFF);
            sizeBytes.append(byte);
        }

        output_file.write(sizeBytes);
        output_file.write("\x04\x00", 2);
        output_file.close();

        QString hash;
        if (domainString == "ConfigProfileDomain")
        {
            hash = QString::fromStdString(calculateSHA1("SysSharedContainerDomain-systemgroup.com.apple.configurationprofiles-" + fileString.toStdString()));
        }
        else if (domainString == "App-PB") {
            hash = QString::fromStdString(calculateSHA1("AppDomain-com.apple.PosterBoard" + fileString.toStdString()));
        }
        else
        {
            hash = QString::fromStdString(calculateSHA1(domainString.toStdString() + "-" + fileString.toStdString()));
        }
        QString newFile = outputDir + "/" + hash;
        copyFile(path, newFile);
    }
    else if (QFileInfo(path).isDir())
    {
        output_file.write("\xFF\xFF\xFF\xFF\xFF\xFF\x41\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xF5\x00\x00\x01\xF5", 24);
        generateRandomHex(output_file);
        output_file.write("\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00", 10);
        output_file.close();

        QDir dir(path);
        for (const QString &entry : dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
        {
            QString filePath = path + "/" + entry;
            processFiles(filePath, domainString, outputDir);
        }
    }
}

bool createDirectory(const QString &dirPath)
{
    try
    {
        if (QDir().mkpath(dirPath))
            return true;
        else
        {
            qDebug() << "Failed to create directory.";
            return false;
        }
    }
    catch (const std::exception &ex)
    {
        qDebug() << "Failed to create directory:" << ex.what();
        return false;
    }
}

bool removeDirectoryIfExists(const QString &dirPath)
{
    if (QFileInfo::exists(dirPath) && QFileInfo(dirPath).isDir())
    {
        try
        {
            if (QDir(dirPath).removeRecursively())
                return true;
            else
                return false;
        }
        catch (const std::exception &ex)
        {
            qDebug() << "Failed to remove directory:" << ex.what();
            return false;
        }
    }
    return false;
}

QString getApplicationData(const std::string udid) {
    QString resulting_plist = "";
    idevice_t device = nullptr;
    lockdownd_client_t lockdown = nullptr;
    lockdownd_service_descriptor_t service = nullptr;
    instproxy_client_t instproxy = nullptr;
    std::string bundle_id = "com.apple.PosterBoard";
    plist_t apps_list = nullptr;

    if (idevice_new_with_options(&device, udid.c_str(), IDEVICE_LOOKUP_USBMUX) != IDEVICE_E_SUCCESS) {
        goto cleanup;
    }
    if (lockdownd_client_new_with_handshake(device, &lockdown, "CreateBackup") != LOCKDOWN_E_SUCCESS) {
        goto cleanup;
    }
    if (lockdownd_start_service(lockdown, "com.apple.mobile.installation_proxy", &service) != LOCKDOWN_E_SUCCESS) {
        goto cleanup;
    }
    if (instproxy_client_new(device, service, &instproxy) != INSTPROXY_E_SUCCESS) {
        goto cleanup;
    }

    // this is required for the goto statements not to error for some reason
    // I love C++ -.-
    if (true) {
        plist_t options = plist_new_dict();

        plist_t attrs = plist_new_array();
        plist_array_append_item(attrs, plist_new_string("CFBundleVersion"));
        plist_array_append_item(attrs, plist_new_string("Container"));
        plist_dict_set_item(options, "ReturnAttributes", attrs);

        const char *app_ids[] = {
            "com.apple.PosterBoard",
            nullptr
        };
        instproxy_error_t lookup_err = instproxy_lookup(instproxy, app_ids, options, &apps_list);
        plist_free(attrs);
        plist_free(options);

        if (lookup_err == INSTPROXY_E_SUCCESS && apps_list) {
            plist_t app = plist_dict_get_item(apps_list, bundle_id.c_str());
            if (app) {
                // this is definitely not a great way of doing it
                // but I am not good enough at C++ to find a better way
                resulting_plist = R"(
    <key>Applications</key>
    <dict>
        <key>com.apple.PosterBoard</key>
        <dict>
            <key>CFBundleIdentifier</key>
            <string>com.apple.PosterBoard</string>
            <key>CFBundleVersion</key>
            <string>)";
                plist_t bundle_ver = plist_dict_get_item(app, "CFBundleVersion");
                uint64_t bundle_length;
                const char *bundle_ver_str = plist_get_string_ptr(bundle_ver, &bundle_length);
                resulting_plist += bundle_ver_str;
                resulting_plist += R"(</string>
            <key>ContainerContentClass</key>
            <string>Data/Application</string>
            <key>Path</key>
            <string>)";
                plist_t container_path = plist_dict_get_item(app, "Container");
                uint64_t cont_length;
                const char *container_path_str = plist_get_string_ptr(container_path, &cont_length);
                resulting_plist += container_path_str;
                resulting_plist += R"(</string>
        </dict>
    </dict>)";
                plist_free(bundle_ver);
                plist_free(container_path);
                plist_free(app);
            }
        }
    }

cleanup:
    if (apps_list)
        plist_free(apps_list);
    if (instproxy)
        instproxy_client_free(instproxy);
    if (service)
        lockdownd_service_descriptor_free(service);
    if (lockdown)
        lockdownd_client_free(lockdown);
    if (device)
        idevice_free(device);

    return resulting_plist;
}

bool CreateBackup::createBackup(const QString& indir, const QString& outdir, const std::string udid)
{
    removeDirectoryIfExists(outdir);
    createDirectory(outdir);

    // NOTE: Manifest.mbdb tracks the locations and SHA1 hashes of each file in the backup
    QFile output_file(outdir + "/Manifest.mbdb");
    if (!output_file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to create output file";
        return false;
    }

    // Manifest.mbdb file header
    QByteArray header = "mbdb\x05\x00";
    output_file.write(header, 6);

    // Close the file after writing the header
    output_file.close();

    // Iterate over all domains
    QDir domainDir(indir);
    bool restorePB = false;
    for (const QString &domainEntry : domainDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString domain = indir + "/" + domainEntry;
        QString domainString = QFileInfo(domain).baseName();
        if (domainString == "App-PB") {
            restorePB = true;
        }

        processFiles(domain, domainString, outdir);
    }

    // Generate Info.plist
    QString infoPlistContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
</dict>
</plist>
)";
    QFile infoPlist(outdir + "/Info.plist");
    if (!infoPlist.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream infoStream(&infoPlist);
    infoStream << infoPlistContent;
    infoPlist.close();

    // Generate Status.plist
    QString statusPlistContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BackupState</key>
	<string>new</string>
	<key>Date</key>
	<date>1970-01-01T00:00:00Z</date>
	<key>IsFullBackup</key>
	<false/>
	<key>SnapshotState</key>
	<string>finished</string>
	<key>UUID</key>
	<string>00000000-0000-0000-0000-000000000000</string>
	<key>Version</key>
	<string>2.4</string>
</dict>
</plist>
)";
    QFile statusPlist(outdir + "/Status.plist");
    if (!statusPlist.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create Status.plist file";
        return false;
    }
    QTextStream statusStream(&statusPlist);
    statusStream << statusPlistContent;
    statusPlist.close();

    // Generate Manifest.plist
    QString apps_list = "";
    if (restorePB) {
        qDebug() << "restorePB";
        apps_list = getApplicationData(udid);
    }
    QString manifestPlistContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BackupKeyBag</key>
	<data>
    VkVSUwAAAAQAAAAFVFlQRQAAAAQAAAABVVVJRAAAABDud41d1b9NBICR1BH9JfVtSE1D
    SwAAACgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAV1JBUAAA
    AAQAAAAAU0FMVAAAABRY5Ne2bthGQ5rf4O3gikep1e6tZUlURVIAAAAEAAAnEFVVSUQA
    AAAQB7R8awiGR9aba1UuVahGPENMQVMAAAAEAAAAAVdSQVAAAAAEAAAAAktUWVAAAAAE
    AAAAAFdQS1kAAAAoN3kQAJloFg+ukEUY+v5P+dhc/Welw/oucsyS40UBh67ZHef5ZMk9
    UVVVSUQAAAAQgd0cg0hSTgaxR3PVUbcEkUNMQVMAAAAEAAAAAldSQVAAAAAEAAAAAktU
    WVAAAAAEAAAAAFdQS1kAAAAoMiQTXx0SJlyrGJzdKZQ+SfL124w+2Tf/3d1R2i9yNj9z
    ZCHNJhnorVVVSUQAAAAQf7JFQiBOS12JDD7qwKNTSkNMQVMAAAAEAAAAA1dSQVAAAAAE
    AAAAAktUWVAAAAAEAAAAAFdQS1kAAAAoSEelorROJA46ZUdwDHhMKiRguQyqHukotrxh
    jIfqiZ5ESBXX9txi51VVSUQAAAAQfF0G/837QLq01xH9+66vx0NMQVMAAAAEAAAABFdS
    QVAAAAAEAAAAAktUWVAAAAAEAAAAAFdQS1kAAAAol0BvFhd5bu4Hr75XqzNf4g0fMqZA
    ie6OxI+x/pgm6Y95XW17N+ZIDVVVSUQAAAAQimkT2dp1QeadMu1KhJKNTUNMQVMAAAAE
    AAAABVdSQVAAAAAEAAAAA0tUWVAAAAAEAAAAAFdQS1kAAAAo2N2DZarQ6GPoWRgTiy/t
    djKArOqTaH0tPSG9KLbIjGTOcLodhx23xFVVSUQAAAAQQV37JVZHQFiKpoNiGmT6+ENM
    QVMAAAAEAAAABldSQVAAAAAEAAAAA0tUWVAAAAAEAAAAAFdQS1kAAAAofe2QSvDC2cV7
    Etk4fSBbgqDx5ne/z1VHwmJ6NdVrTyWi80Sy869DM1VVSUQAAAAQFzkdH+VgSOmTj3yE
    cfWmMUNMQVMAAAAEAAAAB1dSQVAAAAAEAAAAA0tUWVAAAAAEAAAAAFdQS1kAAAAo7kLY
    PQ/DnHBERGpaz37eyntIX/XzovsS0mpHW3SoHvrb9RBgOB+WblVVSUQAAAAQEBpgKOz9
    Tni8F9kmSXd0sENMQVMAAAAEAAAACFdSQVAAAAAEAAAAA0tUWVAAAAAEAAAAAFdQS1kA
    AAAo5mxVoyNFgPMzphYhm1VG8Fhsin/xX+r6mCd9gByF5SxeolAIT/ICF1VVSUQAAAAQ
    rfKB2uPSQtWh82yx6w4BoUNMQVMAAAAEAAAACVdSQVAAAAAEAAAAA0tUWVAAAAAEAAAA
    AFdQS1kAAAAo5iayZBwcRa1c1MMx7vh6lOYux3oDI/bdxFCW1WHCQR/Ub1MOv+QaYFVV
    SUQAAAAQiLXvK3qvQza/mea5inss/0NMQVMAAAAEAAAACldSQVAAAAAEAAAAA0tUWVAA
    AAAEAAAAAFdQS1kAAAAoD2wHX7KriEe1E31z7SQ7/+AVymcpARMYnQgegtZD0Mq2U55u
    xwNr2FVVSUQAAAAQ/Q9feZxLS++qSe/a4emRRENMQVMAAAAEAAAAC1dSQVAAAAAEAAAA
    A0tUWVAAAAAEAAAAAFdQS1kAAAAocYda2jyYzzSKggRPw/qgh6QPESlkZedgDUKpTr4Z
    Z8FDgd7YoALY1g==
    </data>
	<key>Lockdown</key>
	<dict/>
	<key>SystemDomainsVersion</key>
	<string>20.0</string>
	<key>Version</key>
	<string>9.1</string>)";
    manifestPlistContent += apps_list;
    manifestPlistContent += R"(
</dict>
</plist>)";
    QFile manifestPlist(outdir + "/Manifest.plist");
    if (!manifestPlist.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create Manifest.plist file";
        return false;
    }
    QTextStream manifestStream(&manifestPlist);
    manifestStream << manifestPlistContent;
    manifestPlist.close();

    return true;
}
