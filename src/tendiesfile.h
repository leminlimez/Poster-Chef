#ifndef TENDIESFILE_H
#define TENDIESFILE_H

#include <string>
#include <QDir>

class TendiesFile {
public:
    QDir filepath;
    std::string name;
    int descriptor_count;
    bool is_container;
    bool unsafe_container;
    bool loaded;

    TendiesFile(QDir path);
};

#endif // TENDIESFILE_H
