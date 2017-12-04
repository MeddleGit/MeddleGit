#pragma once

#include <QString>
#include <QStringList>
#include <initializer_list>

class Git
{
public:
    QString Cmd(std::initializer_list<QString> args);
    bool SetWorkingDirectory(const QString & dir);
    QString GetWorkingDirectory() { return mWorkingDirectory; }

private:
    QString mWorkingDirectory;

    bool cmd(const QString & workingDirectory, const QStringList & args, QString & output);
};
