#pragma once

#include <QMainWindow>
#include <QMap>

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

private:
    Ui::MainWindow *ui;

    QStringList mLogLines;
    std::vector<QStringList> mHashes;
    QMap<QString, int> mCommitMap;
};
