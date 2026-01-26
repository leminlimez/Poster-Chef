#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "devicemanager.h"
#include "posterboardmanager.h"

#include <QURL>
#include <QDesktopServices>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    showUDID = false;
    ui->setupUi(this);
    ui->pages->setCurrentIndex(static_cast<int>(Page::Home));
    ui->applyStatusLbl->hide();
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
        ui->homePageBtn->setChecked(true);
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

// Tendies Page
void MainWindow::on_exploreBtn_clicked() {
    openWebPage("https://cowabun.ga/wallpapers");
}

void MainWindow::loadTendiesList() {
    if (PosterboardManager::getInstance().importedTendies.empty()) { return; }

    // Clear the layout
    QLayout *layout = ui->tendiesFileList->layout();
    if (layout)
    {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr)
        {
            delete child->widget(); // Remove and delete the widget
            delete child;           // Delete the layout item
        }
        delete layout; // Delete the layout itself
            //        ui->themesCnt->setLayout(nullptr); // Reset the layout pointer
    }

    // Clear the widget contents (if it's a container widget)
    const QObjectList &children = ui->tendiesFileList->children();
    for (QObject *child : children)
    {
        delete child; // Delete each child widget
    }

    // Create a QVBoxLayout to arrange the widgets horizontally
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    if (PosterboardManager::getInstance().importedTendies.empty())
    {
        QVBoxLayout *layout = new QVBoxLayout(ui->tendiesFileList);
        QLabel *label = new QLabel("No tendies files selected, please import one.");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        ui->tendiesFileList->setFixedHeight(150);
        ui->tendiesFileList->setLayout(layout);
        return;
    }

    // Iterate through the tendies objects
    int counter = 0;
    foreach (TendiesFile tendie, PosterboardManager::getInstance().importedTendies) {
        QWidget *widget = new QWidget();
        QHBoxLayout *tendieLayout = new QHBoxLayout();
        tendieLayout->setContentsMargins(0, 0, 0, 3);

        // Add Title
        QToolButton *titleBtn = new QToolButton();
        titleBtn->setIcon(QIcon(tendie.getIcon()));
        titleBtn->setIconSize(QSize(20, 20));
        titleBtn->setText("   " + tendie.name);
        titleBtn->setStyleSheet("QToolButton {\n    background-color: transparent;\n	icon-size: 20px;\n}");
        titleBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        titleBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tendieLayout->addWidget(titleBtn);

        QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        tendieLayout->addSpacerItem(spacer);

        // Add delete button
        QToolButton *delBtn = new QToolButton();
        delBtn->setIcon(QIcon(":/icon/trash.svg"));
        connect(delBtn, &QToolButton::clicked, [this, widget, tendie, counter]()
        {
            widget->deleteLater();
            PosterboardManager::getInstance().importedTendies.erase(PosterboardManager::getInstance().importedTendies.begin() + counter);
            MainWindow::loadTendiesList();
        });
        tendieLayout->addWidget(delBtn);
        widget->setLayout(tendieLayout);
        mainLayout->addWidget(widget);
        counter++;
    }

    // Create a QWidget to act as the container for the scroll area
    QWidget *scrollWidget = new QWidget();
    mainLayout->setAlignment(Qt::AlignTop);

    // Set the main layout (containing all the widgets) on the scroll widget
    scrollWidget->setLayout(mainLayout);

    // Create a QScrollArea to hold the content widget (scrollWidget)
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);       // Allow the content widget to resize within the scroll area
    scrollArea->setFrameStyle(QFrame::NoFrame); // Remove the outline from the scroll area

    // Set the scrollWidget as the content widget of the scroll area
    scrollArea->setWidget(scrollWidget);

    // Set the size policy of the scroll area to expand in both directions
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set the scroll area as the central widget of the main window
    QVBoxLayout *scrollLayout = new QVBoxLayout();
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->addWidget(scrollArea);
    ui->tendiesFileList->setLayout(scrollLayout);
}
void MainWindow::disableResetActions() {
    ui->resetCollectionsChk->setChecked(false);
    ui->resetMercuryChk->setChecked(false);
    ui->resetPhotosChk->setChecked(false);
    ui->resetGalleryCacheChk->setChecked(false);
    for (int i = 0; i < NUM_RESET_MODES; ++i) {
        // Cast the integer back to the enum class type
        ResetMode mode = static_cast<ResetMode>(i);
        PosterboardManager::getInstance().setResetMode(mode, false);
    }
}

void MainWindow::on_importTendiesBtn_clicked() {
    QString selectedFile = QFileDialog::getOpenFileName(nullptr, "Select PosterBoard Files", "", "Zip Files (*.tendies)", nullptr, QFileDialog::ReadOnly);
    if (!selectedFile.isEmpty()) {
        TendiesFile newTendie = TendiesFile(selectedFile);
        if (!newTendie.loaded) {
            qDebug() << "Failed to load tendies file" << selectedFile;
        } else {
            PosterboardManager::getInstance().importedTendies.push_back(newTendie);
            MainWindow::disableResetActions();
            MainWindow::loadTendiesList();
        }
    }
}

// Resetting Page
void MainWindow::on_resetCollectionsChk_clicked(bool checked) {
    PosterboardManager::getInstance().setResetMode(ResetMode::Collections, checked);
}
void MainWindow::on_resetMercuryChk_clicked(bool checked) {
    PosterboardManager::getInstance().setResetMode(ResetMode::MercuryPoster, checked);
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
    ui->skipSetupEnabledLbl->setVisible(checked);
}
void MainWindow::on_supervisionChk_clicked(bool checked) {
    DeviceManager::getInstance().setSupervised(checked);
}
void MainWindow::on_supervisionOrganization_textEdited(const QString &text) {
    DeviceManager::getInstance().setOrganizationName(text.toStdString());
}

void MainWindow::on_applyBtn_clicked() {
    DeviceManager::getInstance().applyTweaks(ui->applyStatusLbl);
}
