// from CowabungaLiteWindows: https://github.com/Avangelista/CowabungaLiteWindows/blob/main/CreateBackup.h

#ifndef CREATEBACKUP_H
#define CREATEBACKUP_H

#include <QString>

bool createDirectory(const QString &dirPath);

class CreateBackup
{
public:
    static bool createBackup(const QString& indir, const QString& outdir);
};


#endif // CREATEBACKUP_H
