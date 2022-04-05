#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    prev = QTime::currentTime();
    fps = 0;
    frame_area = frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;
    motion_detecting_status = false;
    segmentor = cv::createBackgroundSubtractorMOG2(500, 16, true);
    int noise_size = 9;
    cv::getStructuringElement(cv::MORPH_RECT, cv::Size(noise_size, noise_size)).copyTo(kernel);
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    prev = QTime::currentTime();
    fps = 0;
    frame_area = frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;
    motion_detecting_status = false;
    segmentor = cv::createBackgroundSubtractorMOG2(500, 16, true);
    int noise_size = 9;
    cv::getStructuringElement(cv::MORPH_RECT, cv::Size(noise_size, noise_size)).copyTo(kernel);
}

CaptureThread::~CaptureThread() {
}

void CaptureThread::run() {
    running = true;
    cv::VideoCapture cap;
    if (cameraID != -1) {
        cap = cv::VideoCapture(cameraID);
    } else {
        cap = cv::VideoCapture(videoPath.toStdString(), cv::CAP_ANY, {cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_ANY, cv::CAP_PROP_HW_DEVICE, -1});
    }

    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    frame_area = frame_width * frame_height;

    cv::UMat tmp_frame;
    while(running) {
        cap >> tmp_frame;
        if (tmp_frame.empty()) {
            break;
        }

        if(motion_detecting_status) {
            motionDetect(tmp_frame);
        }

        switch (video_saving_status) {
        case STARTING:
            startSavingVideo(tmp_frame);
            break;

        case STARTED:
            video_writer->write(tmp_frame);
            break;

        case STOPPING:
            stopSavingVideo();
            break;

        default:
            break;
        }

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);

        data_lock->lock();
        tmp_frame.copyTo(frame);
        data_lock->unlock();

        calcStats(frame);
        emit frameCaptured(&frame);
    }
    cap.release();
    running = false;
    fps = 0;
}

void CaptureThread::motionDetect(cv::UMat &frame)
{
    cv::UMat fgmask(frame.size(), CV_8UC1, cv::ACCESS_WRITE, cv::USAGE_ALLOCATE_DEVICE_MEMORY);
    segmentor->apply(frame, fgmask);
    if (fgmask.empty()) {
        return;
    }

    cv::threshold(fgmask, fgmask, 25, 255, cv::THRESH_BINARY);

    cv::erode(fgmask, fgmask, kernel);
    cv::dilate(fgmask, fgmask, kernel, cv::Point(-1, -1), 3);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(fgmask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    bool has_motion = contours.size() > 0;
    if (!motion_detected && has_motion) {
        motion_detected = true;
        setVideoSavingStatus(STARTING);
        qDebug() << "new motion detected.";
    } else if (motion_detected && !has_motion) {
        motion_detected = false;
        setVideoSavingStatus(STOPPING);
        qDebug() << "detected motion disappeared.";
    }

    cv::Mat temp_frame(frame.size(), frame.type());
    frame.copyTo(temp_frame);
    cv::Scalar color = cv::Scalar(0, 0, 255);
    for (size_t i = 0; i < contours.size(); i++) {
        cv::Rect rect = cv::boundingRect(contours[i]);
        cv::rectangle(temp_frame, rect, color, 1);
    }
    temp_frame.copyTo(frame);
}

void CaptureThread::startSavingVideo(cv::UMat &firstFrame)
{
    saved_video_name = Utilities::newSavedVideoName();
    QString cover = Utilities::getSavedVideoPath(saved_video_name, "jpg");
    cv::imwrite(cover.toStdString(), firstFrame);
    video_writer = new cv::VideoWriter(
                Utilities::getSavedVideoPath(saved_video_name, "mp4").toStdString(),
                cv::VideoWriter::fourcc('a','v','c','1'),
                fps? fps: 30,
                cv::Size(frame_width,frame_height));
    video_saving_status = STARTED;
}

void CaptureThread::stopSavingVideo()
{
    video_saving_status = STOPPED;
    video_writer->release();
    delete video_writer;
    video_writer = nullptr;
    emit videoSaved(saved_video_name);
}

void CaptureThread::calcStats(const cv::Mat &frame){
    QTime now = QTime::currentTime();
    fps = 1000.0 / prev.msecsTo(now);
    prev = now;

    QMutex data_write;

    std::array<double, 3> mean = {0, 0, 0};
    std::array<double, 3> smean = {0, 0, 0};

    cv::parallel_for_(cv::Range(0, frame_height), [&](const cv::Range& range){
        std::array<double, 3> temp_mean = {0, 0, 0};
        std::array<double, 3> temp_smean = {0, 0, 0};
        for (int row = range.start; row < range.end; row++){
            const cv::Vec3b* p = frame.ptr<cv::Vec3b>(row);
            for (int col = 0; col < frame_width; col++){
                temp_mean[0] += p[col][0];
                temp_mean[1] += p[col][1];
                temp_mean[2] += p[col][2];
                temp_smean[0] += p[col][0] * p[col][0];
                temp_smean[1] += p[col][1] * p[col][1];
                temp_smean[2] += p[col][2] * p[col][2];
            }
        }
        data_write.lock();
        mean[0] += temp_mean[0];
        mean[1] += temp_mean[1];
        mean[2] += temp_mean[2];
        smean[0] += temp_smean[0];
        smean[1] += temp_smean[1];
        smean[2] += temp_smean[2];
        data_write.unlock();
    });

    float std[3];
    for (int i = 0;i < 3;i++) {
        mean[i] /= frame_area;
        std[i] = std::sqrt(smean[i] / frame_area - mean[i] * mean[i]);
    }

    emit statsChanged(fps, cv::Vec3f(mean[0], mean[1], mean[2]),  cv::Vec3f(std[0], std[1], std[2]));
}
