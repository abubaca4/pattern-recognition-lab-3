QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    #russian win console error text
    QMAKE_EXTRA_TARGETS += before_build makefilehook

    makefilehook.target = $(MAKEFILE)
    makefilehook.depends = .beforebuild

    PRE_TARGETDEPS += .beforebuild

    before_build.target = .beforebuild
    before_build.depends = FORCE
    before_build.commands = chcp 1251

    #opencv
    win32:CONFIG(release, debug|release): OPENCV_INSTALL_PATH = C:\opencv\build
    else:win32:CONFIG(debug, debug|release): OPENCV_INSTALL_PATH = C:\opencv_d\build

    win32:CONFIG(release, debug|release): OPENCV_VERSION = 455
    else:win32:CONFIG(debug, debug|release): OPENCV_VERSION = 455d

    INCLUDEPATH += \
        $${OPENCV_INSTALL_PATH}\include

    DEPENDPATH += \
        $${OPENCV_INSTALL_PATH}\include

    LIBS += \
        -L$${OPENCV_INSTALL_PATH}\lib \
        -lopencv_core$${OPENCV_VERSION} \
        -lopencv_imgproc$${OPENCV_VERSION} \
        -lopencv_highgui$${OPENCV_VERSION} \
        -lopencv_imgcodecs$${OPENCV_VERSION} \
        -lopencv_videoio$${OPENCV_VERSION} \
        -lopencv_video$${OPENCV_VERSION} \
        -lopencv_calib3d$${OPENCV_VERSION} \
        -lopencv_photo$${OPENCV_VERSION} \
        -lopencv_features2d$${OPENCV_VERSION}

}
