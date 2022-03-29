#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , statusLabel(QLabel("Gazer is Ready"))
{
    ui->setupUi(this);

#ifdef GAZER_USE_QT_CAMERA
    QGridLayout* layout = static_cast<QGridLayout*>(ui->centralwidget->layout());
    videoWidget = new QVideoWidget;
    layout->addWidget(videoWidget, 0, 0, 2, 1);
#else
    capturer = nullptr;
    imageScene = new QGraphicsScene();
    ui->graphicsView->setScene(imageScene);
    data_lock = new QMutex();
#endif

    ui->statusbar->addPermanentWidget(&statusLabel);
}

MainWindow::~MainWindow()
{
    delete ui;

#ifdef GAZER_USE_QT_CAMERA
    delete videoWidget;
#else
    delete imageScene;
    delete data_lock;
#endif
}

void MainWindow::on_action_Camera_Information_triggered()
{
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QString info = QString("Available cameras: \n");

    foreach (const QCameraDevice &cameraInfo, cameras) {
        info += QString(" - %1: %2\n").arg(cameraInfo.id(), cameraInfo.description());
    }
    QMessageBox::information(this, "Cameras", info);
}

void MainWindow::on_action_Open_Camera_triggered()
{
    int camID = 0;

#ifdef GAZER_USE_QT_CAMERA
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    camera = new QCamera(cameras[camID]);
    QMediaCaptureSession* mediaCaptureSession = new QMediaCaptureSession;
    mediaCaptureSession->setCamera(camera);
    mediaCaptureSession->setVideoOutput(videoWidget);
    camera->start();
#else
    if(capturer != nullptr) {
        // if a thread is already running, stop it
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::statsChanged, this, &MainWindow::updateStats);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }
    capturer = new CaptureThread(camID, data_lock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::statsChanged, this, &MainWindow::updateStats);
    capturer->start();
#endif
    statusLabel.setText(QString("Capturing Camera %1").arg(camID));
}

void MainWindow::on_action_Exit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionOpen_Video_triggered()
{

}

#ifndef GAZER_USE_QT_CAMERA
void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();
    QImage frame(
                currentFrame.data,
                currentFrame.cols,
                currentFrame.rows,
                currentFrame.step,
                QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);
    imageScene->clear();
    ui->graphicsView->resetTransform();
    imageScene->addPixmap(image);
    imageScene->update();
    ui->graphicsView->setSceneRect(image.rect());
}

void MainWindow::updateStats(float fps, cv::Vec3f mean, cv::Vec3f std){
    statusLabel.setText(QString::asprintf("FPS: %.2f, mean: (%.2f, %.2f, %.2f), std: (%.2f, %.2f, %.2f)", fps, mean[0], mean[1], mean[2], std[0], std[1], std[2]));
}
#endif
