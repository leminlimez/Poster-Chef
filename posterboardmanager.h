#ifndef POSTERBOARDMANAGER_H
#define POSTERBOARDMANAGER_H

#include <set>

enum class ResetMode {
    Collections = 0,
    Photos = 1,
    GalleryCache = 2
};

class PosterboardManager
{
public:
    static PosterboardManager& getInstance();

    void setResetMode(ResetMode mode, bool active);

private:
    PosterboardManager();
    ~PosterboardManager();

    bool reset_modes[3] = {false, false, false};
};

#endif // POSTERBOARDMANAGER_H
