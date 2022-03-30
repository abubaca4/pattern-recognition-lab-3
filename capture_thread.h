#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QTime>

#include "opencv2/opencv.hpp"

#include <array>
#include <vector>

#include "utilities.h"

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();

    enum VideoSavingStatus {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };

    void setRunning(bool run) {running = run; };
    void setVideoSavingStatus(VideoSavingStatus status) {video_saving_status = status; };
    void setMotionDetectingStatus(bool status) {
        motion_detecting_status = status;
        motion_detected = false;
        if (video_saving_status != STOPPED) video_saving_status = STOPPING;
    }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void statsChanged(float fps, cv::Vec3f mean, cv::Vec3f std);
    void videoSaved(QString name);

private:
    void calcStats(const cv::Mat &frame);
    void startSavingVideo(cv::UMat &firstFrame);
    void stopSavingVideo();

    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    QTime prev;
    float fps;
    int frame_width, frame_height, frame_area;
    // video saving
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;

    void motionDetect(cv::UMat &frame);
    bool motion_detecting_status;
    bool motion_detected;
    cv::Ptr<cv::BackgroundSubtractorMOG2> segmentor;
    cv::UMat kernel;
};

#endif // CAPTURE_THREAD_H
