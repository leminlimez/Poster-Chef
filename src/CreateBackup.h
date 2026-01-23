// from CowabungaLiteWindows: https://github.com/Avangelista/CowabungaLiteWindows/blob/main/CreateBackup.h

#ifndef CREATEBACKUP_H
#define CREATEBACKUP_H

#include <QString>
#include <string>

bool createDirectory(const QString &dirPath);

class CreateBackup
{
public:
    static bool createBackup(const QString& indir, const QString& outdir, const std::string udid);
};


#endif // CREATEBACKUP_H
