#include "tendiesfile.h"

#include "utils.h"

#include <minizip/unzip.h>
#include <QRandomGenerator>

TendiesFile::TendiesFile(const QString &path) : filepath(path),
    name(QFileInfo(path).fileName()),
    descriptorCount(0),
    isContainer(false),
    unsafeContainer(false),
    loaded(false) {
    readFromZip();
}

void TendiesFile::readFromZip() {
    unzFile zip = unzOpen(filepath.toUtf8().constData());
    if (!zip) { return; }

    if (unzGoToFirstFile(zip) != UNZ_OK) {
        unzClose(zip);
        return;
    }

    do {
        unz_file_info info;
        char filename[512];

        if (unzGetCurrentFileInfo(
                zip,
                &info,
                filename,
                sizeof(filename),
                nullptr,
                0,
                nullptr,
                0) != UNZ_OK)
            break;

        QString option = QString::fromUtf8(filename).toLower();

        // Skip macOS metadata
        if (option.contains("__macosx/"))
            continue;

        // Container detection
        if (option.contains("container")) {
            isContainer = true;

            if (option.contains("pbfposterextensiondatastoresqlitedatabase.sqlite3")) {
                unsafeContainer = true;
            }
        }

        // Descriptor counting
        if (option.contains("descriptor/")) {
            QString item = option.section("descriptor/", 1);
            if (item.count('/') == 1 && item.endsWith('/')) {
                descriptorCount++;
            }
        } else if (option.contains("descriptors/")) {
            QString item = option.section("descriptors/", 1);
            if (item.count('/') == 1 && item.endsWith('/')) {
                descriptorCount++;
            }
        }

    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    loaded = true;
}

QString TendiesFile::getIcon() const
{
    if (isContainer) {
        return QStringLiteral(":/icon/shippingbox.svg");
    } else if (descriptorCount == 1) {
        return QStringLiteral(":/icon/photo.svg");
    } else {
        return QStringLiteral(":/icon/photo-stack.svg");
    }
}

QString TendiesFile::extract(const QString &outputDir) const {
    QString zipOutput = QDir(outputDir).filePath(Utils::randomString(10));
    QDir().mkpath(zipOutput);

    unzFile zip = unzOpen(filepath.toUtf8().constData());
    if (!zip)
        return QString();

    if (unzGoToFirstFile(zip) != UNZ_OK) {
        unzClose(zip);
        return QString();
    }

    do {
        unz_file_info info;
        char filename[512];

        if (unzGetCurrentFileInfo(
                zip,
                &info,
                filename,
                sizeof(filename),
                nullptr,
                0,
                nullptr,
                0) != UNZ_OK)
            break;

        QString filePath = QDir(zipOutput).filePath(QString::fromUtf8(filename));

        if (filePath.endsWith('/')) {
            QDir().mkpath(filePath);
            continue;
        }

        QDir().mkpath(QFileInfo(filePath).path());

        if (unzOpenCurrentFile(zip) != UNZ_OK)
            continue;

        QFile outFile(filePath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            unzCloseCurrentFile(zip);
            continue;
        }

        std::vector<char> buffer(8192);
        int bytesRead = 0;

        while ((bytesRead = unzReadCurrentFile(zip, buffer.data(), buffer.size())) > 0) {
            outFile.write(buffer.data(), bytesRead);
        }

        outFile.close();
        unzCloseCurrentFile(zip);

    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    return zipOutput;
}
