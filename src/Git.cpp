#include "Git.h"

#include <QProcess>

QString Git::Cmd(std::initializer_list<QString> args)
{
    QStringList qargs;
    for(auto arg : args)
        qargs.append(arg);
    QProcess p;
    //p.setWorkingDirectory("/Users/duncan/go/src/code.gitea.io/gitea");
    p.setWorkingDirectory("/Users/duncan/Projects/git");
    //p.setWorkingDirectory("/Users/duncan/Projects/AgileJourney");
    //p.setWorkingDirectory("/Users/duncan/Projects/legacy-homebrew");
    //p.setWorkingDirectory("/Users/duncan/Projects/radare2");
    //p.setWorkingDirectory("/Users/duncan/Projects/rails");
    //p.setWorkingDirectory("/Users/duncan/Projects/tensorflow");
    p.start("/usr/local/bin/git", qargs);
    p.waitForFinished(-1);
    return QString(p.readAllStandardOutput());
}
