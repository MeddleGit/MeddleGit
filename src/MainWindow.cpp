#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Git.h"

//Resources:
//https://github.com/tclh123/commits-graph
//https://stackoverflow.com/questions/1057564/pretty-git-branch-graphs
//https://github.com/tclh123/commits-graph/blob/master/git/commits_graph.py
//http://www.qtcentre.org/threads/27777-Customize-QListWidgetItem-how-to

#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QGraphicsItem>

#include <queue>
#include <tuple>

#ifdef Q_OS_WIN
#include <windows.h>
#include "resource.h"
#endif //Q_OS_WIN

// Beware when adding menu items on OS X: https://stackoverflow.com/a/31028590/1806760

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Windows hack for setting the icon in the taskbar.
#ifdef Q_OS_WIN
    HICON hIcon = LoadIconW(GetModuleHandleW(0), MAKEINTRESOURCE(IDI_ICON1));
    SendMessageW((HWND)winId(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    DestroyIcon(hIcon);
#endif //Q_OS_WIN
}

MainWindow::~MainWindow()
{
    delete ui;
}

template<class Key, class Value>
struct Test
{
    std::vector<int> a;

    std::vector<int>::iterator works()
    {
        return a.begin();
    }

    std::vector<std::pair<Key, Value>> b;

    typename std::vector<std::pair<Key, Value>>::iterator why()
    {
        return b.begin();
    }
};

#include <QTime>
#include <QElapsedTimer>

void MainWindow::on_actionTest_triggered()
{
    //Run git command
    QElapsedTimer timer;
    timer.start();
    QFile f("/Users/duncan/Projects/git.txt");
    f.open(QFile::ReadOnly);
    mOutput = f.readAll();
    f.close();
    //mOutput = Git::Cmd({"log", "--graph", "--date-order", "--no-color", "--all", "--pretty=format:%H %P"});

    qDebug() << "git command finished" << timer.elapsed() << "ms";
    timer.restart();

    //Parse the output
    mLogLines.clear();
    mLogLines.reserve(1000000);
    mHashes.clear();
    mHashes.reserve(1000000);
    mCommitMap.clear();
    mCommitMap.reserve(1000000);
    int maxWidth = -1, maxLine = -1;
    for(int i = 0, linepos = 0, linecount = 0; i < mOutput.length(); i++)
    {
        if(mOutput.at(i) == '\n' || i + 1 == mOutput.length())
        {
            QStringRef line(&mOutput, linepos, i - linepos + (i + 1 == mOutput.length()));
            QStringRef log;
            QVector<QStringRef> hashes;
            for(int j = 0; j < line.length(); j++)
            {
                auto ch = line.at(j).toLatin1();
                if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'))
                {
                    log = line.mid(0, j);
                    hashes = line.mid(j).split(' ', QString::SkipEmptyParts);
                    break;
                }
            }

            if(log.isEmpty())
                log = line;

            if(log.length() > maxWidth)
                maxWidth = log.length(), maxLine = i;

            if(!hashes.empty())
                mCommitMap.add(hashes.first(), linecount);

            mLogLines.append(log);
            mHashes.push_back(std::move(hashes));

            linepos = i + 1, linecount++;
        }
    }
    mCommitMap.sort();
    /*QString hash("d455a3696c72283923e6870e9e4fe1daa861d7cd");
    auto found = mCommitMap.find(QStringRef(&hash));
    if(found != mCommitMap.end())
        qDebug() << found->first << found->second;
    else
        qDebug() << ":'(";*/

    setWindowTitle(QString("grid: %1x%2 (%3)").arg(maxWidth).arg(mLogLines.length()).arg(maxLine));

    qDebug() << "parsed output" << timer.elapsed() << "ms";
    timer.restart();

    //Set the textual output
    {
        QString output;
        output.reserve(mOutput.size());
        for(int i = 0; i < mLogLines.length(); i++)
        {
            output += mLogLines[i];
            if(!mHashes[i].isEmpty())
            {
                output += ' ';
                output += mHashes[i].first().left(8);
            }
            output += '\n';
        }
        ui->plainTextLog->setPlainText(output);
    }

    qDebug() << "set text output" << timer.elapsed() << "ms";
    timer.restart();

    //Graph grid structure
    struct Grid
    {
        Grid(int width, int height)
            : mWidth(width),
              mHeight(height),
              mData(width * height, ' ') { }

        char get(int x, int y)
        {
            auto idx = map(x, y);
            if(idx >= 0 && idx < mWidth * mHeight)
                return mData[idx];
            return ' ';
        }

        void set(int x, int y, char c)
        {
            auto idx = map(x, y);
            if(idx >= 0 && idx < mWidth * mHeight)
                mData[idx] = c;
        }

        char operator[](QPoint p)
        {
            return get(p.x(), p.y());
        }

        int width()
        {
            return mWidth;
        }

        int height()
        {
            return mHeight;
        }

    private:
        int mWidth, mHeight;
        std::vector<char> mData;

        int map(int x, int y)
        {
            return x + y * mWidth;
        }
    };

    //Fill the grid
    std::vector<QPoint> commits;
    commits.reserve(mLogLines.length());
    Grid grid(maxWidth, mLogLines.length());
    for(int y = 0; y < mLogLines.length(); y++)
    {
        const QStringRef & line = mLogLines.at(y);
        for(int x = 0; x < line.length(); x++)
        {
            auto ch = line.at(x).toLatin1();
            if(ch == '*')
                commits.push_back(QPoint(x, y));
            grid.set(x, y, ch);
        }
    }

    qDebug() << "fill the grid" << timer.elapsed() << "ms";
    timer.restart();

    size_t paths = 0, goodpaths = 0;

    //Walk the paths from each commit to their parents
    qDebug() << "total commits: " << commits.size();
    std::map<std::tuple<char, char, int, int>, std::pair<int, int>> usedCases;
    std::vector<QPoint> queue;
    queue.reserve(100);
    for(size_t commitIdx = 0; commitIdx < commits.size(); commitIdx++)
    {
        auto commit = commits[commitIdx];
        std::vector<QPoint> parents;
        parents.reserve(2);
        auto & hashes = mHashes[commit.y()];
        //qDebug() << QString("%1/%2 (%3)").arg(commitIdx + 1).arg(commits.size()).arg(hashes.first());
        //qDebug() << count << hashes.first() << commit;
        int maxY = -1;
        for(int i = 1; i < hashes.length(); i++)
        {
            auto found = mCommitMap.find(hashes[i]);
            if(found == mCommitMap.end())
            {
                qDebug() << "this shouldn't happen (parent hash)..." << hashes[i];
                return;
            }
            int parentY = found->second;
            int parentX = -1;
            for(int j = 0; j < mLogLines[parentY].length(); j++)
            {
                if(mLogLines[parentY].at(j) == '*')
                {
                    parentX = j;
                    break;
                }
            }
            if(parentX < 0)
            {
                qDebug() << "this shouldn't happen (parentX)...";
                break;
            }
            maxY = qMax(maxY, parentY);
            parents.push_back(QPoint(parentX, parentY));
        }

        if(!parents.size())
            continue;

        goodpaths += parents.size();

        /*qDebug() << "parents:";
        for(auto & parent : parents)
            qDebug() << parent;*/

        struct QPointLess
        {
            bool operator()(const QPoint & a, const QPoint & b) const
            {
                return std::make_pair(a.x(), a.y()) < std::make_pair(b.x(), b.y());
            }
        };

        //Walk the path of the dragon
        std::map<QPoint, QPoint, QPointLess> previous;
        std::vector<QPoint> reached;
        queue.clear();
        queue.push_back(commit);
        auto oldpaths = paths;
        std::vector<std::vector<QPoint>> test;
        while(!queue.empty())
        {
            QPoint p = queue.back();
            queue.pop_back();
            auto cur = grid[p];
            //qDebug() << "step" << cur << p;

            auto parentIdx = [&]()
            {
                for(int i = 0; i < int(parents.size()); i++)
                    if(parents.at(i) == p)
                        return i;
                return -1;
            }();

            if(parentIdx != -1)
            {
                //qDebug() << "found!";
                reached.push_back(p);
                /*std::vector<QPoint> path;
                do
                {
                    path.push_back(p);
                    p = previous[p];
                } while(p != commit);
                path.push_back(p);
                std::reverse(path.begin(), path.end());
                test.push_back(path);*/
                paths++;
                //commitpaths[commitIdx][parentIdx].push_back(path);
                /*QString out;
                for(auto item : path)
                    out += QString("(%1, %2) ").arg(item.x()).arg(item.y());
                qDebug() << out.trimmed();*/
                continue;
            }

            if(cur == '*' && p != commit)
            {
                //qDebug() << "abort";
                continue;
            }

            auto peekNeighbor = [&](int dx, int dy)
            {
                return grid.get(p.x() + dx, p.y() + dy);
            };
            auto neighbor = [&](int dx, int dy, char check)
            {
                QPoint np = p + QPoint(dx, dy);
                if(np.y() > maxY) //TODO: turn on optimization if the code is stable
                    return false;
                if(grid[np] != check)
                {
                    //usedCases[std::make_tuple(cur, check, dx, dy)].second++;
                    return false;
                }
                queue.push_back(np);
                //previous[np] = p;
                //usedCases[std::make_tuple(cur, check, dx, dy)].first++;
                return true;
            };

            auto prevsize = queue.size();
            Q_UNUSED(prevsize);

            switch(cur)
            {
            case '|':
                if(neighbor(+0, +1, '|'))
                    break;
                if(neighbor(+1, +1, '\\'))
                    break;
                if(neighbor(-1, +1, '/'))
                    break;
                if(neighbor(+0, +1, '*'))
                    break;
                break;

            case '\\':
                if(neighbor(+0, +1, '/'))
                    break;
                if(neighbor(+1, +1, '*'))
                    break;
                if(neighbor(+1, +1, '|'))
                    break;
                if(neighbor(+1, +1, '\\'))
                    break;
                if(queue.size() - prevsize == 0)
                    neighbor(+0, +1, '_');
                break;

            case '/':
                if(peekNeighbor(-1, +0) == '|' && neighbor(-2, +0, '_'))
                    break;
                if(neighbor(-2, +1, '/'))
                    break;
                if(neighbor(-1, +1, '|'))
                    break;
                if(neighbor(-1, +1, '/'))
                    break;
                if(neighbor(-1, +1, '*'))
                    break;
                if(neighbor(-1, +1, '_'))
                    break;
                if(peekNeighbor(-1, +0) == ' ' && peekNeighbor(-2, +0) == '|' && neighbor(-3, +0, '_'))
                    break;
                break;

            case '*':
                neighbor(+0, +1, '*');
                neighbor(+0, +1, '|');
                neighbor(+1, +1, '\\');
                neighbor(-1, +1, '/');
                if(peekNeighbor(+1, +0) == '-')
                {
                    int i = 1;
                    while(true)
                    {
                        auto ch = peekNeighbor(i + 1, 0);
                        if(ch == '-' || ch == '.')
                        {
                            neighbor(i + 2, 1, '\\');
                            i += 2;
                        }
                        else
                            break;
                    }
                }
                //neighbor(+1, +0, '-');
                break;

            case '_':
                if(neighbor(-2, +0, '_'))
                    break;
                if(neighbor(-2, +1, '/'))
                    break;

                //This is a very tricky case, the underscore in this configuration can connect to any '|' branch on it's left
                //Probably a bug in git: https://stackoverflow.com/questions/15250012/unexpected-underscore-in-git-log-graph-output
                for(int i = 1; i < p.x(); i+=2)
                    neighbor(-i + 1, +0, '|');
                break;

            case '-':
                neighbor(+1, +0, '-');
                neighbor(+1, +1, '\\');
                neighbor(+1, +0, '.');
                break;

            case '.':
                neighbor(+1, +1, '\\');
                break;

            default:
                qDebug() << "this shouldn't happen " << cur;
                break;
            }

            /*if(queue.size() - prevsize > 1)
            {
                switch(cur)
                {
                case '*':
                case '-':
                case '.':
                case '_': //TODO: find a solution for this
                    break;
                default:
                    qDebug() << "branching" << cur << p;
                    return;
                }
            }*/

            /*if(queue.size() - prevsize == 0)
            {
                qDebug() << "uncaught cases!" << cur << p;
            }*/
        }

        for(size_t i = 0; i < parents.size(); i++)
        {
            if(std::find(reached.begin(), reached.end(), parents[i]) == reached.end())
            {
                qDebug() << "not reached: " << parents[i].y() - commit.y() << parents[i] << hashes.first();
            }
        }

        if(paths - oldpaths != parents.size())
        {
            qDebug() << ":(" << commit << hashes.first() << paths - oldpaths << '\n';
            auto blub = [](const std::vector<QPoint> & path)
            {
                QString result;
                for(size_t i = 0; i < path.size(); i++)
                {
                    if(i)
                        result += " ";
                    result += QString("(%1, %2)").arg(path[i].x()).arg(path[i].y());
                }
                return result;
            };
            for(auto path : test)
                qDebug() << blub(path);
        }

        //qDebug() << "finished";
    }

    //if(usedCases.size() != 27)
    {
        qDebug() << "case usage:" << usedCases.size();
        for(auto it : usedCases)
        {
            auto data = it.first;
            qDebug() << std::get<0>(data) << std::get<1>(data) << std::get<2>(data) << std::get<3>(data) << "hits:" << it.second.first << "misses:" << it.second.second;
        }
    }

    qDebug() << "parse paths" << timer.elapsed() << "ms";
    timer.restart();

    qDebug() << "total paths:" << paths;
    qDebug() << "total good paths:" << goodpaths;

    QGraphicsScene* scene = new QGraphicsScene(this);
    ui->graph->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    auto transx = [](int x)
    {
        return 10 + x * 6;
    };
    auto transy = [](int y)
    {
        return 10 + y * 14;
    };

    scene->addRect(0, 0, transx(maxWidth), transy(mLogLines.length()), QPen(Qt::black));

    for(QPoint & commit : commits)
    {
        auto & hashes = mHashes[commit.y()];
        auto hash = hashes[0];
        if(hash.isEmpty())
        {
            qDebug() << "this shouldn't happen (commit)...";
            return;
        }
        int cx = transx(commit.x()), cy = transy(commit.y());
        for(int j = 1; j < hashes.length(); j++)
        {
            auto found = mCommitMap.find(hashes[j]);
            if(found == mCommitMap.end())
            {
                qDebug() << "this shouldn't happen (parent)..." << hash << hashes[j];
            }
            else
            {
                //TODO: store parent X
                int parentY = found->second;
                int parentX = -1;
                for(int j = 0; j < mLogLines[parentY].length(); j++)
                {
                    if(mLogLines[parentY].at(j) == '*')
                    {
                        parentX = j;
                        break;
                    }
                }
                if(parentX < 0)
                {
                    qDebug() << "this shouldn't happen (parentX)...";
                    break;
                }
                int px = transx(parentX), py = transy(parentY);
                scene->addLine(cx, cy, px, py, QPen(Qt::blue));
            }
        }
    }

    for(QPoint & commit : commits)
    {
        int cx = transx(commit.x()), cy = transy(commit.y());
        scene->addRect(cx - 2, cy - 2, 4, 4, QPen(Qt::red), QBrush(Qt::red));
    }

    ui->graph->setScene(scene);

    qDebug() << "draw scene" << timer.elapsed() << "ms";
    timer.restart();
}

void MainWindow::on_plainTextLog_cursorPositionChanged()
{
    QTextEdit::ExtraSelection highlight;
    highlight.cursor = ui->plainTextLog->textCursor();
    highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
    highlight.format.setBackground( Qt::green );

    QList<QTextEdit::ExtraSelection> extras;
    extras << highlight;
    ui->plainTextLog->setExtraSelections(extras);

    if(mHashes.empty())
        return;

    auto line = ui->plainTextLog->textCursor().blockNumber();
    auto & hashes = mHashes.at(line);
    if(hashes.empty())
    {
        ui->plainTextCommit->clear();
        return;
    }

    QString parents;
    for(int i = 1; i < hashes.length(); i++)
        parents.append(hashes[i]), parents.push_back('\n');
    ui->plainTextCommit->setPlainText(QString("%1: %2\nparent(s):\n%3").arg(line).arg(hashes[0].toString()).arg(parents));
}

void MainWindow::on_pushButton_clicked()
{
    ui->pushButton->setText("PushMeSenpai");

    QGraphicsScene* scene = new QGraphicsScene(this);
    //ui->graph->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    scene->addRect(0, 0, 400, 1600, QPen(Qt::black));

    struct Grid
    {
        Grid(int width, int height)
        {
            GridArray.resize(width*height);
            this->w = width;
            this->h = height;
        }

        int w;
        int h;
        std::vector<char> GridArray;

        int map(int x, int y)
        {
            return x + y * this->w;
        }

    };

    Grid grid(7, 12);

    Grid blub(1, 1);

    blub.map(0, 0);

    grid.GridArray[grid.map(1, 2)] = '*';

    scene->addEllipse(8, 8, 4, 4, QPen(QColor(255, 150, 0)), QBrush(QColor(255, 150, 0)));

    ui->graph->setScene(scene);
}
