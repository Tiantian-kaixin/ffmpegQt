//
// Created by TIANTIAN on 2023/2/17.
//
#pragma once

#ifndef UNTITLED_FFMPEGTHREAD_H
#define UNTITLED_FFMPEGTHREAD_H

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avio.h"
#include "libavutil/imgutils.h"
}
#include <QString>
#include <QDebug>
#include <QImage>
#include "QThread"

class FfmpegThread: public QThread {
Q_OBJECT
public:
    FfmpegThread(QObject *parent = nullptr);
    ~FfmpegThread();
    void setPath(const QString &src);
    Q_SIGNAL void nextFrames(uchar *bits, int width, int height);

protected:
    void run();

private:
    QString audioPath;
};


#endif //UNTITLED_FFMPEGTHREAD_H
