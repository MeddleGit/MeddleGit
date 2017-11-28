#include "MainWindow.h"
#include "ui_MainWindow.h"

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
