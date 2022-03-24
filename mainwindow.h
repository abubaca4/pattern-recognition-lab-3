#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "opencv2/opencv.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Camera_Information_triggered();
    void on_action_Open_Camera_triggered();
    void on_action_Exit_triggered();
    void on_actionOpen_Video_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
