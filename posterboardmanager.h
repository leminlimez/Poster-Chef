#ifndef POSTERBOARDMANAGER_H
#define POSTERBOARDMANAGER_H

#include <QString>
#include <QDir>

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

    void setResetMode(ResetMode mode, bool active);
    void createResetModeFiles(QString path);

private:
    PosterboardManager();
    ~PosterboardManager();

    bool reset_modes[4] = {false, false, false, false};

    QString getPBFolderPath();
    void createEmptyFile(QString directory, QString filename);
};

#endif // POSTERBOARDMANAGER_H
