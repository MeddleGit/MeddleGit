#include "Git.h"

#include <QProcess>
#include <QDir>
#include <QDebug>

QString Git::Cmd(std::initializer_list<QString> args)
{
    QString output;
    if(!cmd(mWorkingDirectory, QStringList(args), output))
        qDebug() << "error: git" << QStringList(args);
    return output;
}

bool Git::SetWorkingDirectory(const QString & dir)
{
    QString output;
    if(!cmd(dir, QStringList({"rev-parse", "--show-toplevel"}), output))
        return false;
    mWorkingDirectory = QDir::toNativeSeparators(output.trimmed());
    return true;
}

bool Git::cmd(const QString & workingDirectory, const QStringList & args, QString & output)
{
    QProcess p;
    p.setWorkingDirectory(workingDirectory);
#ifdef Q_OS_OSX
    //p.setWorkingDirectory("/Users/duncan/go/src/code.gitea.io/gitea");
    //p.setWorkingDirectory("/Users/duncan/Projects/git");
    //p.setWorkingDirectory("/Users/duncan/Projects/AgileJourney");
    //p.setWorkingDirectory("/Users/duncan/Projects/legacy-homebrew");
    //p.setWorkingDirectory("/Users/duncan/Projects/radare2");
    //p.setWorkingDirectory("/Users/duncan/Projects/rails");
    //p.setWorkingDirectory("/Users/duncan/Projects/tensorflow");
    p.start("/usr/local/bin/git", args);
#else
    p.start("git", args);
#endif //Q_OS_OSX
    p.waitForFinished(-1);
    if(p.exitCode() == 0)
    {
        output = p.readAllStandardOutput();
        return true;
    }
    else
    {
        output = p.readAllStandardError();
        return false;
    }
}
