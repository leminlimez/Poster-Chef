#ifndef TENDIESFILE_H
#define TENDIESFILE_H

#include <QDir>

class TendiesFile {
public:
    QString filepath;
    QString name;
    int descriptorCount;
    bool isContainer;
    bool unsafeContainer;
    bool loaded;

    TendiesFile(const QString &path);
    QString getIcon() const;
    QString extract(const QString &outputDir) const;

private:
    void readFromZip();
    QString randomString(int length) const;
};

#endif // TENDIESFILE_H
