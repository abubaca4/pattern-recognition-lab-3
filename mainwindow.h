#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMessageBox>
#include <QLabel>

#include "opencv2/opencv.hpp"

#ifdef GAZER_USE_QT_CAMERA
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QCamera>
#else
#include <QGraphicsScene>
#include "capture_thread.h"
#endif

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

#ifdef GAZER_USE_QT_CAMERA
#else
    void updateFrame(cv::Mat *mat);
#endif
private:
    Ui::MainWindow *ui;  

    QLabel statusLabel;

#ifdef GAZER_USE_QT_CAMERA
    QCamera *camera;
    QVideoWidget *videoWidget;
#else
    cv::Mat currentFrame;
    // for capture thread
    QMutex *data_lock;
    CaptureThread *capturer;
    QGraphicsScene *imageScene;
#endif
};
#endif // MAINWINDOW_H
