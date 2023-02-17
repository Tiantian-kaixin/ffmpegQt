//
// Created by TIANTIAN on 2023/2/17.
//

#ifndef UNTITLED_FFMPEGUTIL_H
#define UNTITLED_FFMPEGUTIL_H

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

class FfmpegUtil {
public:
    int videoToImage(const QString &audioPath,const QString&outputDir);
};


#endif //UNTITLED_FFMPEGUTIL_H
