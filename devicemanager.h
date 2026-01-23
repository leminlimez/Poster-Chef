#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "utils.h"
#include <QLabel>
#include <string>

struct DeviceInfo {
    std::string UUID;
    std::string Name;
    Version Version;
};

class DeviceManager
{
public:
    static DeviceManager& getInstance();

    std::vector<DeviceInfo> loadDevices();

    int getCurrentDeviceIndex();
    void setCurrentDeviceIndex(int index);
    void resetCurrentDevice();

    const std::optional<std::string> getCurrentUUID() const;
    const std::optional<Version> getCurrentVersion() const;
    const std::optional<std::string> getCurrentName() const;

    bool getSkipSetup();
    void setSkipSetup(bool enabled);
    bool isSupervised();
    void setSupervised(bool enabled);
    std::string getOrganizationName();
    void setOrganizationName(std::string name);

    bool isDeviceAvailable();

    const QString getWorkspace() const;

    void applyTweaks(QLabel* statusLabel);

private:
    DeviceManager();
    ~DeviceManager();

    static DeviceManager instance;

    std::optional<DeviceInfo> currentDevice;
    int currentDeviceIndex;
    std::vector<DeviceInfo> devices;
    bool deviceAvailable;

    // Preferences
    bool skipSetup;
    bool supervised;
    std::string organizationName;

    bool restoreBackupToDevice(const std::string&, const std::string&);
};

#endif // DEVICEMANAGER_H
