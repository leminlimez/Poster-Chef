#include "utils.h"

#include <QDir>
#include <QDirIterator>
#include <QRandomGenerator>

QString Utils::randomString(int length)
{
    static const char chars[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    QString result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        int idx = QRandomGenerator::global()->bounded(int(sizeof(chars) - 1));
        result.append(chars[idx]);
    }

    return result;
}

QString Utils::generate_uuid_v4() {
    uint32_t data[4];
    for (auto &d : data) {
        d = QRandomGenerator::global()->generate();
    }

    // Version 4
    data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
    // Variant 10xx
    data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

    return QStringLiteral("%1-%2-%3-%4-%5")
        .arg(data[0], 8, 16, QLatin1Char('0'))
        .arg(data[1] >> 16, 4, 16, QLatin1Char('0'))
        .arg(data[1] & 0xFFFF, 4, 16, QLatin1Char('0'))
        .arg(data[2] >> 16, 4, 16, QLatin1Char('0'))
        .arg(data[2] & 0xFFFF, 4, 16, QLatin1Char('0'))
        .append(QString("%1").arg(data[3], 8, 16, QLatin1Char('0')))
        .toUpper();
}

bool Utils::createDirectory(const QString &dirPath)
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

bool Utils::copyDirectory(QString source, QString dest) {
    QDir().mkpath(dest);

    auto iterator = QDirIterator(source, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        iterator.next();

        auto sourceFilePath = iterator.filePath();
        auto relativeFilePath = iterator.fileInfo().absoluteFilePath().replace(source, "");
        auto destinationFilePath = dest + relativeFilePath;

        // Create the destination directory path if necessary
        QDir().mkpath(QFileInfo(destinationFilePath).absolutePath());

        // Copy the file or directory
        if (iterator.fileInfo().isFile()) {
            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                //                return false;
            }
        } else if (iterator.fileInfo().isDir()) {
            if (!QDir().mkdir(destinationFilePath)) {
                //                return false;
            }
        }
    }

    return true;
}
