#include "posterboardmanager.h"

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
