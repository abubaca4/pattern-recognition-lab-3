#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QMessageBox>
#include <QLabel>
#include <QStandardItem>
#include <QFileDialog>

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
    void on_pushButton_clicked();

#ifdef GAZER_USE_QT_CAMERA
#else
    void updateFrame(cv::Mat *mat);
    void updateStats(float fps, cv::Vec3f mean, cv::Vec3f std);
    void appendSavedVideo(QString name);
#endif

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    QLabel statusLabel;
    QStandardItemModel *list_model;

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

    void populateSavedList();
};
#endif // MAINWINDOW_H
