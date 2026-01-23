#include "devicemanager.h"

#include "CreateBackup.h"
#include "posterboardmanager.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/restore.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

DeviceManager::DeviceManager() {
    // Constructor implementation
    currentDevice = std::nullopt;
    currentDeviceIndex = 0;
    deviceAvailable = false;

    skipSetup = true;
    supervised = false;
}

DeviceManager &DeviceManager::getInstance()
{
    static DeviceManager instance;
    return instance;
}

DeviceManager::~DeviceManager()
{
    // Destructor implementation
}

std::vector<DeviceInfo> DeviceManager::loadDevices()
{
    auto devices = std::vector<DeviceInfo>();

    char **device_list = nullptr;
    int device_count = 0;
    idevice_get_device_list(&device_list, &device_count);

    for (int i = 0; i < device_count; ++i)
    {
        char *uuid = device_list[i];

        if (uuid)
        {
            idevice_t device = nullptr;
            idevice_error_t error = idevice_new_with_options(&device, uuid, IDEVICE_LOOKUP_USBMUX);
            lockdownd_client_t client = nullptr;
            lockdownd_error_t lockdownd_error = lockdownd_client_new_with_handshake(device, &client, "DeviceManager");

            if (error == IDEVICE_E_SUCCESS && lockdownd_error == LOCKDOWN_E_SUCCESS)
            {
                plist_t device_info = nullptr;
                lockdownd_error_t info_error = lockdownd_get_value(client, nullptr, "ProductVersion", &device_info);

                if (info_error == LOCKDOWN_E_SUCCESS)
                {
                    char *device_version = nullptr;
                    plist_get_string_val(device_info, &device_version);

                    plist_t device_name_plist = nullptr;
                    lockdownd_error_t name_error = lockdownd_get_value(client, nullptr, "DeviceName", &device_name_plist);

                    if (name_error == LOCKDOWN_E_SUCCESS && device_name_plist && plist_get_node_type(device_name_plist) == PLIST_STRING)
                    {
                        char *device_name = nullptr;
                        plist_get_string_val(device_name_plist, &device_name);

                        DeviceInfo deviceInfo;
                        deviceInfo.UUID = uuid;
                        deviceInfo.Name = (device_name != nullptr) ? device_name : "";
                        deviceInfo.Version = (device_version != nullptr) ? Version(device_version) : Version();

                        devices.push_back(deviceInfo);

                        if (device_name)
                            free(device_name);
                    }

                    if (device_name_plist)
                        plist_free(device_name_plist);

                    if (device_version)
                        free(device_version);
                }

                if (device_info)
                    plist_free(device_info);
            }

            if (client)
                lockdownd_client_free(client);

            if (device)
                idevice_free(device);
        }
    }

    if (device_list)
        idevice_device_list_free(device_list);

    this->devices = devices;

    // If the same device is still there, set it as current
    if (this->currentDevice)
    {
        for (size_t i = 0; i < devices.size(); ++i)
        {
            if (devices.at(i).UUID == this->currentDevice->UUID)
            {
                setCurrentDeviceIndex(i);
                return devices;
            }
        }
    }

    // Otherwise, reset the current device
    resetCurrentDevice();
    return devices;
}

int DeviceManager::getCurrentDeviceIndex()
{
    return this->currentDeviceIndex;
}

bool DeviceManager::isDeviceAvailable()
{
    return this->deviceAvailable;
}

void DeviceManager::setCurrentDeviceIndex(int index)
{
    currentDeviceIndex = index;
    currentDevice = devices.at(index);
    // version check
    if (currentDevice->Version >= Version(17))
    {
        this->deviceAvailable = true;
    }
    else
    {
        this->deviceAvailable = false;
    }

}

void DeviceManager::resetCurrentDevice()
{
    if (this->devices.empty())
    {
        this->currentDeviceIndex = 0;
        this->currentDevice.reset();
        this->deviceAvailable = false;
    }
    else
    {
        DeviceManager::setCurrentDeviceIndex(0);
    }
}

const std::optional<std::string> DeviceManager::getCurrentUUID() const
{
    if (this->currentDevice)
    {
        return this->currentDevice->UUID;
    }
    else
    {
        return std::nullopt;
    }
}

const std::optional<Version> DeviceManager::getCurrentVersion() const
{
    if (this->currentDevice)
    {
        return this->currentDevice->Version;
    }
    else
    {
        return std::nullopt;
    }
}

const std::optional<std::string> DeviceManager::getCurrentName() const
{
    if (this->currentDevice)
    {
        return this->currentDevice->Name;
    }
    else
    {
        return std::nullopt;
    }
}

// Preferences
bool DeviceManager::getSkipSetup() {
    return this->skipSetup;
}
void DeviceManager::setSkipSetup(bool enabled) {
    this->skipSetup = enabled;
}
bool DeviceManager::isSupervised() {
    return this->supervised;
}
void DeviceManager::setSupervised(bool enabled) {
    this->supervised = enabled;
}


// Applying tweaks
const QString DeviceManager::getWorkspace() const
{
    // Get the destination directory path
    auto workspaceDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Workspace/";
    return workspaceDir;
}

void DeviceManager::applyTweaks(QLabel* statusLabel) {
    statusLabel->show();
    auto workspacePath = DeviceManager::getWorkspace();
    statusLabel->setText("Cleaning up folder...");
    auto workspace = QDir(workspacePath);
    if (workspace.exists()) {
        workspace.removeRecursively();
    }
    if (!createDirectory(workspacePath)) {
        statusLabel->setText("Failed to create workspace at " + workspacePath);
        return;
    }
    // Create tweak files
    statusLabel->setText("Generating files...");
    PosterboardManager::getInstance().createResetModeFiles(workspacePath);

    // Generate the backup
    statusLabel->setText("Generating backup...");
    auto backupDirectoryPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Backup";
    auto udid_val = getCurrentUUID();
    if (udid_val.has_value()) {
        CreateBackup::createBackup(workspacePath, backupDirectoryPath, *udid_val);
    } else {
        // Make error condition
        statusLabel->setText("No udid found!");
        return;
    }
    statusLabel->setText("Restoring backup to device...");

    auto success = DeviceManager::restoreBackupToDevice(*DeviceManager::getCurrentUUID(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString());
    if (success) {
        statusLabel->setText("Done!");
    } else {
        statusLabel->setText("Failed.");
    }
}

bool DeviceManager::restoreBackupToDevice(const std::string& udid, const std::string& backupDirectory) {
    QStringList arguments;
    arguments << "--idevicebackup2" << "-u" << QString::fromStdString(udid) << "-s" << "Backup" << "restore" << "--system" << "--skip-apps" << QString::fromStdString(backupDirectory);

    QProcess process;
    process.start(QCoreApplication::applicationFilePath(), arguments);
    process.waitForFinished(-1);

    QByteArray output = process.readAllStandardOutput();
    QByteArray errorOutput = process.readAllStandardError();

    // Split the output into lines using '\n' as the separator
    // AAA Fix using \r\n
    QString sep;
#if _WIN32
    sep = "\r\n";
#else
    sep = "\n";
#endif
    QStringList outputLines = QString(output).split(sep);

    // Get the last two lines of the output
    QString lastLine;
    QString secondLastLine;
    if (outputLines.size() >= 3) {
        lastLine = outputLines.at(outputLines.size() - 2);
        secondLastLine = outputLines.at(outputLines.size() - 3);
    } else {
        lastLine = output;
        secondLastLine = errorOutput;
    }
    qDebug() << "lastLine:" << lastLine;

    if (lastLine == "Restore Successful.")
    {
        QMessageBox::information(nullptr, "Success!", "All done! Your device will now restart.\n\nRemember to turn back on Find My!\n\nImportant: If you are presented with a setup, select \"Customize\" > \"Don't transfer apps and data\" and your phone should return to the homescreen as normal.");
        return true;
    }
    QMessageBox detailsMessageBox;
    detailsMessageBox.setWindowTitle("Error!");
    detailsMessageBox.setIcon(QMessageBox::Critical);
    detailsMessageBox.setText(lastLine + "\n" + secondLastLine);
    detailsMessageBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsMessageBox.setDetailedText(errorOutput + "\n" + output);
    detailsMessageBox.exec();
    return false;
}
