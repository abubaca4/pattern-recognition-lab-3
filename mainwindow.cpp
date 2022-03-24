#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Camera_Information_triggered()
{

}

void MainWindow::on_action_Open_Camera_triggered()
{

}

void MainWindow::on_action_Exit_triggered()
{

}

void MainWindow::on_actionOpen_Video_triggered()
{

}
