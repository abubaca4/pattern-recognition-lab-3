#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStandardPaths>
#include <QDir>

class Utilities
{
public:
    static QString getDataPath();
    static QString newSavedVideoName();
    static QString getSavedVideoPath(QString name, QString postfix);
};

#endif // UTILITIES_H
