#ifndef PLISTMANAGER_H
#define PLISTMANAGER_H

#include <string>
#include <plist/plist.h>
#include <QString>

class PlistManager
{
public:
    static void setPlistValue(const QString& plistPath, const std::string& key, plist_t& value);
private:
    static bool isBinaryPlist(const std::vector<char>& data);
};

#endif // PLISTMANAGER_H
