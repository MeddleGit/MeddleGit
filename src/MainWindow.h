#pragma once

#include <QMainWindow>
#include <QMap>

#include "VectorMap.h"
#include "Git.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionTest_triggered();
    void on_plainTextLog_cursorPositionChanged();
    void on_pushButton_clicked();
    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;

    Git mGit;
    QString mOutput;
    QVector<QStringRef> mLogLines;
    std::vector<QVector<QStringRef>> mHashes;
    VectorMap<QStringRef, int> mCommitMap;
    QString mTitle;
};
