#ifndef POSTERBOARDMANAGER_H
#define POSTERBOARDMANAGER_H

#include "tendiesfile.h"

#include <vector>
#include <QString>
#include <QDir>

#define NUM_RESET_MODES (4)

enum class ResetMode {
    Collections = 0,
    MercuryPoster = 1,
    Photos = 2,
    GalleryCache = 3
};

class PosterboardManager
{
public:
    static PosterboardManager& getInstance();

    std::vector<TendiesFile> importedTendies;

    void setResetMode(ResetMode mode, bool active);
    bool createResetModeFiles(QString path);
    void generateTendiesFiles(QString path);

private:
    PosterboardManager();
    ~PosterboardManager();

    bool reset_modes[NUM_RESET_MODES] = {false, false, false, false};

    QString getPBFolderPath();
    void createEmptyFile(QString directory, QString filename);
    void updateRandomizedID(const QFileInfo &file, const int randomizedID);
    void recursiveModify(const QString &path, const QString &resultPath);
    void recursiveModify(const QString &path, const QString &resultPath, const bool isAdding, const int randomizedID);
};

#endif // POSTERBOARDMANAGER_H
