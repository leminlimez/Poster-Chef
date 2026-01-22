#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "devicemanager.h"
#include "posterboardmanager.h"

#include <QURL>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    showUDID = false;
    ui->setupUi(this);
    ui->pages->setCurrentIndex(static_cast<int>(Page::Home));
    MainWindow::refreshDevices();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Sidebar

void MainWindow::updateInterfaceForNewDevice() {
    // TEMP: Hide apply button to activate only when there are selected wallpapers
    MainWindow::updatePhoneInfo();
    showUDID = false;
}

void MainWindow::refreshDevices()
{
    // Clear existing items in the combobox
    ui->devicePicker->clear();

    // Load devices
    auto devices = DeviceManager::getInstance().loadDevices();

    if (devices.empty())
    {
        ui->devicePicker->setEnabled(false);
        ui->devicePicker->addItem(QString("None"), QVariant::fromValue(NULL));
        ui->pages->setCurrentIndex(static_cast<int>(Page::Home));
        toggle_applyBtn_visibility(false);
        // ui->homePageBtn->setChecked(true);

        // hide all pages
        ui->tendiesPageBtn->hide();
        ui->videoPageBtn->hide();
        ui->resettingPageBtn->hide();
    }
    else
    {
        if (ui->pages->currentIndex() == static_cast<int>(Page::Home) || ui->pages->currentIndex() == static_cast<int>(Page::Settings)) {
            toggle_applyBtn_visibility(false);
        }
        ui->devicePicker->setEnabled(true);
        // Populate the combobox with device names
        for (auto &device : devices)
        {
            auto deviceName = QString("%1").arg(QString::fromStdString(device.Name));
            ui->devicePicker->addItem(deviceName, QVariant::fromValue(device.UUID));
        }

        // show all pages
        ui->tendiesPageBtn->show();
        ui->videoPageBtn->show();
        ui->resettingPageBtn->show();
    }

    // Update selected device
    ui->devicePicker->setCurrentIndex(DeviceManager::getInstance().getCurrentDeviceIndex());

    // Update interface
    MainWindow::updateInterfaceForNewDevice();
}

void MainWindow::on_refreshBtn_clicked()
{
    refreshDevices();
}

void MainWindow::on_devicePicker_activated(int index)
{
    DeviceManager::getInstance().setCurrentDeviceIndex(index);
    MainWindow::updateInterfaceForNewDevice();
}

// Home Page
void MainWindow::updatePhoneInfo()
{
    auto name = DeviceManager::getInstance().getCurrentName();
    if (name)
    {
        ui->phoneNameLbl->setText(QString::fromStdString(*name));
    }
    else
    {
        ui->phoneNameLbl->setText("No Device");
    }
    auto version = DeviceManager::getInstance().getCurrentVersion();
    if (version)
    {
        if (DeviceManager::getInstance().isDeviceAvailable())
        {
            ui->phoneVersionLbl->setText("<a style=\"text-decoration:none; color: white;\" href=\"#\">iOS " + QString::fromStdString(version->toString()) + " <span style=\"color: #32d74b;\">Supported!</span></a>");
        }
        else
        {
            ui->phoneVersionLbl->setText("<a style=\"text-decoration:none; color: white;\" href=\"#\">iOS " + QString::fromStdString(version->toString()) + " <span style=\"color: #ff453a;\">Not Supported.</span></a>");
        }
    }
    else
    {
        ui->phoneVersionLbl->setText("Please connect a device.");
    }
}

void MainWindow::toggle_applyBtn_visibility(bool visible) {
    ui->bottomBar->setVisible(visible);
    ui->applyBarLine->setVisible(visible);
}

// Top Bar Pages
void MainWindow::on_homePageBtn_clicked() {
    ui->pages->setCurrentIndex(static_cast<int>(Page::Home));
    toggle_applyBtn_visibility(false);
}
void MainWindow::on_tendiesPageBtn_clicked() {
    ui->pages->setCurrentIndex(static_cast<int>(Page::Tendies));
    toggle_applyBtn_visibility(true);
}
void MainWindow::on_videoPageBtn_clicked() {
    ui->pages->setCurrentIndex(static_cast<int>(Page::Video));
    toggle_applyBtn_visibility(true);
}
void MainWindow::on_resettingPageBtn_clicked() {
    ui->pages->setCurrentIndex(static_cast<int>(Page::Resetting));
    toggle_applyBtn_visibility(true);
}
void MainWindow::on_settingsPageBtn_clicked() {
    ui->pages->setCurrentIndex(static_cast<int>(Page::Settings));
    toggle_applyBtn_visibility(false);
}

void openWebPage(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

// Home Page
void MainWindow::on_bigNuggetBtn_clicked() {
    openWebPage("https://cowabun.ga");
}
void MainWindow::on_discordBtn_clicked() {
    openWebPage("https://discord.gg/gWtzTVhMvh");
}
void MainWindow::on_phoneVersionLbl_linkActivated(const QString &link)
{
    auto uuid = DeviceManager::getInstance().getCurrentUUID();
    if (uuid)
    {
        showUDID = !showUDID;
        if (showUDID) {
            ui->phoneVersionLbl->setText("<a style=\"text-decoration:none; color: white\" href=\"#\">" + QString::fromStdString(*uuid) + "</a>");
        } else {
            MainWindow::updatePhoneInfo();
        }
    }
}

void MainWindow::on_leminTwitterBtn_clicked() {
    openWebPage("https://twitter.com/LeminLimez");
}
void MainWindow::on_leminGithubBtn_clicked() {
    openWebPage("https://github.com/leminlimez");
}
void MainWindow::on_leminKoFiBtn_clicked() {
    openWebPage("https://ko-fi.com/leminlimez");
}

void MainWindow::on_exploreBtn_clicked() {
    openWebPage("https://cowabun.ga/wallpapers");
}

// Resetting Page
void MainWindow::on_resetCollectionsChk_clicked(bool checked) {
    PosterboardManager::getInstance().setResetMode(ResetMode::Collections, checked);
}
void MainWindow::on_resetPhotosChk_clicked(bool checked) {
    PosterboardManager::getInstance().setResetMode(ResetMode::Photos, checked);
}
void MainWindow::on_resetGalleryCacheChk_clicked(bool checked) {
    PosterboardManager::getInstance().setResetMode(ResetMode::GalleryCache, checked);
}

// Settings Page
void MainWindow::on_skipSetupChk_clicked(bool checked) {
    DeviceManager::getInstance().setSkipSetup(checked);
    ui->supervisionChk->setVisible(checked);
    ui->supervisionOrganization->setVisible(checked);
}
void MainWindow::on_supervisionChk_clicked(bool checked) {
    DeviceManager::getInstance().setSupervised(checked);
}
