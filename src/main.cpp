#include "mainwindow.h"

extern "C" {
#include "proc_modules/idevicebackup2.h"
};

#include <string>
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    if (argc > 1 && std::string(argv[1]) == "--idevicebackup2") {
        strncpy(argv[1], argv[1] + 2, 14);
        argv[1][14] = 0;
        idevicebackup2_main(argc-1, argv+1);
    } else {
        QApplication a(argc, argv);

        QTranslator translator;
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = "CrispyPoster_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                a.installTranslator(&translator);
                break;
            }
        }
        MainWindow w;
        w.show();
        return a.exec();
    }
}
