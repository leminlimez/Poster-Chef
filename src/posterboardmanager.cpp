#include "posterboardmanager.h"

#include "CreateBackup.h"
#include "plistmanager.h"
#include "utils.h"

#include <fstream>
#include <QFileInfo>
#include <QFile>
#include <QRandomGenerator>
#include <QString>
#include <QTextStream>

PosterboardManager::PosterboardManager() {
    // Constructor implementation
    importedTendies = std::vector<TendiesFile>();
}

PosterboardManager &PosterboardManager::getInstance()
{
    static PosterboardManager instance;
    return instance;
}

PosterboardManager::~PosterboardManager() {
    // Destructor implementation
}

void PosterboardManager::setResetMode(ResetMode mode, bool active) {
    reset_modes[static_cast<int>(mode)] = active;
}

QString PosterboardManager::getPBFolderPath() {
    // name it PB to prevent file length limits on Windows
    QString fullpath = "App-PB/Library/Application Support/PRBPosterExtensionDataStore/61";
    return fullpath;
}

void PosterboardManager::createEmptyFile(QString directory, QString filename) {
    if (!Utils::createDirectory(directory)) {
        // TODO: add error here
        return;
    }
    QDir parentDir = QDir(directory);
    QString filepath = parentDir.absoluteFilePath(filename);
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.writeRawData("", 0);
        file.close();
    } else {
        qDebug() << "Error: Unable to write the icon data to file.";
    }
}

bool PosterboardManager::createResetModeFiles(QString path) {
    QString pbpath = getPBFolderPath();
    bool isCreated = false;
    if (reset_modes[static_cast<int>(ResetMode::Collections)]) {
        // reset Collections folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.WallpaperKit.CollectionsPoster", "descriptors");
        isCreated = true;
    }
    if (reset_modes[static_cast<int>(ResetMode::MercuryPoster)]) {
        // reset MercuryPoster folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.MercuryPoster", "descriptors");
        isCreated = true;
    }
    if (reset_modes[static_cast<int>(ResetMode::Photos)]) {
        // reset Suggested Photos folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.PhotosUIPrivate.PhotosPosterProvider", "descriptors");
        isCreated = true;
    }
    if (reset_modes[static_cast<int>(ResetMode::GalleryCache)]) {
        // reset GalleryCache folder
        createEmptyFile(path + pbpath, "GalleryCache");
        isCreated = true;
    }
    return isCreated;
}

void PosterboardManager::updateRandomizedID(const QFileInfo &file, const int randomizedID) {
    QString filename = file.fileName().toLower();
    if (filename.contains("com.apple.posterkit.provider.descriptor.identifier")) {
        QFile qfile(file.absoluteFilePath());
        if (!qfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file for writing:" << file.absoluteFilePath();
            return;
        }
        QTextStream out(&qfile);
        out << randomizedID;
        qfile.close();
    } else if (filename.contains("com.apple.posterkit.provider.contents.userinfo")) {
        auto node = plist_new_int(randomizedID);
        PlistManager::setPlistValue(file.absoluteFilePath(), "wallpaperRepresentingIdentifier", node);
    } else if (filename.contains("wallpaper.plist")) {
        auto node = plist_new_int(randomizedID);
        PlistManager::setPlistValue(file.absoluteFilePath(), "identifier", node);
    }
}

void PosterboardManager::recursiveModify(const QString &path, const QString &resultPath) {
    recursiveModify(path, resultPath, false, 0);
}
void PosterboardManager::recursiveModify(const QString &path, const QString &resultPath, const bool isAdding, const int randomizedID) {
    QDir parentDir = QDir(path);
    QFileInfoList entries = parentDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        QString filename = entry.fileName().toLower();
        if (filename.contains("__macosx")) {
            continue;
        }
        if (entry.isDir()) {
            if (isAdding) {
                recursiveModify(entry.absoluteFilePath(), resultPath, isAdding, randomizedID);
            } else {
                if (filename.contains("container")) {
                    // move all items into App-PB
                    QString pbpath = resultPath + "App-PB";
                    Utils::copyDirectory(entry.absoluteFilePath(), pbpath); // TODO: add error handling
                } else if (filename.contains("descriptor")) {
                    QString extension;
                    if (filename.contains("video") || filename.contains("photos")) {
                        extension = "com.apple.PhotosUIPrivate.PhotosPosterProvider";
                    } else if (filename.contains("mercury")) {
                        extension = "com.apple.MercuryPoster";
                    } else {
                        extension = "com.apple.WallpaperKit.CollectionsPoster";
                    }
                    QString pbpath = resultPath + getPBFolderPath() + "/Extensions/" + extension + "/descriptors/";
                    if (!Utils::createDirectory(pbpath)) {
                        // TODO: add error here
                        return;
                    }
                    // TODO: add handling of ordered descriptors

                    // add every folder/wallpaper in the directory
                    QFileInfoList descriptors = QDir(entry.absoluteFilePath()).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                    for (const QFileInfo &descriptor : descriptors) {
                        int id = QRandomGenerator::global()->bounded(9999, 99999);
                        qDebug() << "Current id:" << id;
                        recursiveModify(descriptor.absoluteFilePath(), resultPath, true, id);
                        QString destpath = pbpath + Utils::generate_uuid_v4();
                        qDebug() << "Resultant path:" << destpath;
                        // TODO: add error handling for if path already exists
                        QDir().rename(descriptor.absoluteFilePath(), destpath);
                    }
                }
            }
        } else if (isAdding && entry.isFile()) {
            // randomize ids
            updateRandomizedID(entry, randomizedID);
        }
    }
}

void PosterboardManager::generateTendiesFiles(QString path) {
    if (importedTendies.empty()) { return; }
    QString extraction_path = path + "../TendiesExtraction";
    if (QDir(extraction_path).exists()) {
        QDir(extraction_path).removeRecursively();
    }
    Utils::createDirectory(extraction_path); // TODO: Add error handling
    foreach (TendiesFile tendie, importedTendies) {
        QString result_path = tendie.extract(extraction_path);
        recursiveModify(result_path, path);
    }
    // remove the directory
    QDir(extraction_path).removeRecursively();
}
