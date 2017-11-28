#include "MainWindow.h"
#include "ui_MainWindow.h"

//Beware when adding menu items: https://stackoverflow.com/a/31028590/1806760

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
