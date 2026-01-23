#include "posterboardmanager.h"

#include "CreateBackup.h"

#include <QFile>

PosterboardManager::PosterboardManager() {
    // Constructor implementation
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
    QString fullpath = "PB/Library/Application Support/PRBPosterExtensionDataStore/61";
    return fullpath;
}

void PosterboardManager::createEmptyFile(QString directory, QString filename) {
    if (!createDirectory(directory)) {
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

void PosterboardManager::createResetModeFiles(QString path) {
    QString pbpath = getPBFolderPath();
    if (reset_modes[static_cast<int>(ResetMode::Collections)]) {
        // reset Collections folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.WallpaperKit.CollectionsPoster", "descriptors");
    }
    if (reset_modes[static_cast<int>(ResetMode::MercuryPoster)]) {
        // reset MercuryPoster folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.MercuryPoster", "descriptors");
    }
    if (reset_modes[static_cast<int>(ResetMode::Photos)]) {
        // reset Suggested Photos folder
        createEmptyFile(path + pbpath + "/Extensions/com.apple.PhotosUIPrivate.PhotosPosterProvider", "descriptors");
    }
    if (reset_modes[static_cast<int>(ResetMode::GalleryCache)]) {
        // reset GalleryCache folder
        createEmptyFile(path + pbpath, "GalleryCache");
    }
}
