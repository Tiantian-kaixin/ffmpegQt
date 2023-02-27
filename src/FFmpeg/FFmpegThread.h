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
#include "libswresample/swresample.h"
}
#include <QString>
#include <QDebug>
#include <QImage>
#include "QThread"
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>

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
    QString videoPath;
    AVFormatContext *pFormatCtx;
    int videoIndex = -1;
    int audioIndex = -1;
    void playAudio();
    bool initAudio(int SampleRate);
    std::shared_ptr<QAudioOutput> audio_;
    int64_t getCurrentTime();

    void openVideo();
    AVCodecContext* getCodecCtx(AVMediaType mediaType, int &avIndex);
    void readFrame(AVCodecContext* pCodecCtx);
    void phaseFrame(AVCodecContext* pCodecCtx);
};


#endif //UNTITLED_FFMPEGTHREAD_H
