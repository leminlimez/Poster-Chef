#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class Page {
    Home = 0,
    Tendies = 1,
    Video = 2,
    Resetting = 3,
    Settings = 4
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Topbar
    void on_refreshBtn_clicked();
    void on_devicePicker_activated(int index);
    void on_homePageBtn_clicked();
    void on_tendiesPageBtn_clicked();
    void on_videoPageBtn_clicked();
    void on_resettingPageBtn_clicked();
    void on_settingsPageBtn_clicked();

    // Home Page
    void on_bigNuggetBtn_clicked();
    void on_discordBtn_clicked();
    void on_phoneVersionLbl_linkActivated(const QString &link);

    void on_leminTwitterBtn_clicked();
    void on_leminGithubBtn_clicked();
    void on_leminKoFiBtn_clicked();

    void on_exploreBtn_clicked();

    // Reset Page
    void on_resetCollectionsChk_clicked(bool checked);
    void on_resetPhotosChk_clicked(bool checked);
    void on_resetGalleryCacheChk_clicked(bool checked);

    // Settings Page
    void on_skipSetupChk_clicked(bool checked);
    void on_supervisionChk_clicked(bool checked);

private:
    Ui::MainWindow *ui;

    bool showUDID;

    // Utilities
    void updateInterfaceForNewDevice();

    // Topbar
    void refreshDevices();

    // Home
    void updatePhoneInfo();

    // Bottom bar
    void toggle_applyBtn_visibility(bool visible);
};
#endif // MAINWINDOW_H
