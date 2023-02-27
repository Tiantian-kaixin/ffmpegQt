//
// Created by TIANTIAN on 2023/2/17.
//

#include "FFmpegThread.h"

FfmpegThread::FfmpegThread(QObject *parent) {

}

FfmpegThread::~FfmpegThread() {

}
void FfmpegThread::setPath(const QString &src) {
    videoPath = src;
}

int64_t FfmpegThread::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    return value.count();
}

void FfmpegThread::run() {
    // 注册FFMpeg的库
    av_register_all();
    /*** （一）打开音视频流并获取音视频流信息 ***/
    openVideo();

    /*** （二）查找视频流位置以及查找并打开视频解码器 ***/
    AVCodecContext * vCodecCtx = getCodecCtx(AVMediaType::AVMEDIA_TYPE_VIDEO, videoIndex);// 视频流编码结构
    AVCodecContext * aCodecCtx = getCodecCtx(AVMediaType::AVMEDIA_TYPE_AUDIO, audioIndex);
    initAudio(44100);
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, videoPath.toLocal8Bit(), 0); // 此函数打印输入或输出的详细信息
    printf("-------------------------------------------------\n");

    /*** （三）视频解码的同时处理图片像素数据 ***/
    // 创建帧结构，此函数仅分配基本结构空间，图像数据空间需通过av_malloc分配
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    // 创建动态内存,创建存储图像数据的空间（av_image_get_buffer_size获取一帧图像需要的大小）
    unsigned char *out_buffer = (unsigned char *)av_malloc((size_t)av_image_get_buffer_size(AV_PIX_FMT_RGB32,
         vCodecCtx->width, vCodecCtx->height, 1));
    // 存储一帧像素数据缓冲区
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
         AV_PIX_FMT_RGB32, vCodecCtx->width, vCodecCtx->height, 1);
    // 初始化img_convert_ctx结构
    struct SwsContext *img_convert_ctx; // 主要用于视频图像的转换
    img_convert_ctx = sws_getContext(vCodecCtx->width, vCodecCtx->height, vCodecCtx->pix_fmt,
         vCodecCtx->width, vCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);

    // audio
    AVFrame *audioFrame = av_frame_alloc();
    avcodec_parameters_to_context(aCodecCtx, pFormatCtx->streams[audioIndex]->codecpar);  // 初始化AVCodecContext
    SwrContext *swrctx = nullptr;
    int sampleRate = aCodecCtx->sample_rate > 44100 ? aCodecCtx->sample_rate : 44100;
    swrctx = swr_alloc_set_opts(swrctx, av_get_default_channel_layout(2), AV_SAMPLE_FMT_S16, sampleRate,
                                aCodecCtx->channel_layout, aCodecCtx->sample_fmt, aCodecCtx->sample_rate, NULL, NULL);
    swr_init(swrctx);
    audio_->stop();
    QIODevice *io = audio_->start();
    // av_read_frame读取一帧未解码的数据
    auto *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    int64_t start_time = getCurrentTime();
    while (true) {
        // 如果是视频数据
        int ret = av_read_frame(pFormatCtx, packet);
        if (ret < 0) {
            break;
        }
        if (packet->stream_index == videoIndex) {
            // 解码一帧视频数据
            int got_picture;
            ret = avcodec_decode_video2(vCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("Decode Error.\n");
                return ;
            }
            if (got_picture) {
                sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, vCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);
                emit nextFrames((uchar*)pFrameRGB->data[0], vCodecCtx->width, vCodecCtx->height);
                // sleep time
                AVRational time_base = pFormatCtx->streams[videoIndex]->time_base;
                AVRational time_base_q = {1, AV_TIME_BASE};
                int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);
                int64_t now_time = (getCurrentTime() - start_time) * 1000;
                if (pts_time > now_time) {
                    QThread::usleep(pts_time - now_time);
                }
            }
        } else if (packet->stream_index == audioIndex) {
            // 解码一帧数据
            ret = avcodec_send_packet(aCodecCtx, packet);
            av_packet_unref(packet);
            if (ret != 0) {
                continue;
            }
            avcodec_receive_frame(aCodecCtx, audioFrame);
            uint8_t *data[2] = {0};
            int byteCnt = audioFrame->nb_samples * 2 * 2;

            unsigned char *pcm = new uint8_t[byteCnt];  // frame->nb_samples*2*2表示分配样本数据量*两通道*每通道2字节大小

            data[0] = pcm;  // 输出格式为AV_SAMPLE_FMT_S16(packet类型),所以转换后的LR两通道都存在data[0]中

            swr_convert(swrctx, data, audioFrame->nb_samples,                    // 输出
                              (const uint8_t **)audioFrame->data, audioFrame->nb_samples);  // 输入
            io->write((const char *)pcm, byteCnt);
            delete[] pcm;
        }
        av_free_packet(packet);
    }

    /*** （四）最后要释放申请的内存空间 ***/
    av_frame_free(&audioFrame);
    swr_free(&swrctx);
    avcodec_free_context(&aCodecCtx);
    sws_freeContext(img_convert_ctx); // 释放一个SwsContext
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(vCodecCtx);
    avformat_close_input(&pFormatCtx);
}

void FfmpegThread::openVideo() {
    // 初始化AVFormatContext
    pFormatCtx = avformat_alloc_context(); // 存储音视频封装格式中包含的信息
    // 打开音视频流
    if (avformat_open_input(&pFormatCtx, videoPath.toLocal8Bit().data(), nullptr, nullptr) != 0)
    {
        printf("Couldn't open input stream.\n");
        return;
    }
    // 获取音视频流数据信息
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        printf("Couldn't find stream information.\n");
        return;
    }
}

AVCodecContext* FfmpegThread::getCodecCtx(AVMediaType mediaType, int &avIndex) {
    // 查找视频流的起始索引位置（nb_streams表示视音频流的个数）
    for (int i = 0; i < (int)pFormatCtx->nb_streams; i++)
    {
        // 查找到视频流时退出循环
        AVMediaType type = pFormatCtx->streams[i]->codec->codec_type;
        if (type == mediaType) { // 判断是否为视频流
            avIndex = i;
        }
    }
    if (avIndex == -1) {
        printf("Didn't find a video stream.\n");
        return nullptr;
    }
    // 查找视频解码器
    AVCodecContext* avCodecCtx = pFormatCtx->streams[avIndex]->codec; // 获取视频流编码结构
    AVCodec *pCodec = avcodec_find_decoder(avCodecCtx->codec_id); // 视频解码器
    if (pCodec == nullptr) {
        printf("Codec not found.\n");
        return nullptr;
    }
    // 打开解码器
    if (avcodec_open2(avCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.\n");
        return nullptr;
    }
    return avCodecCtx;
}

void FfmpegThread::playAudio() {
    int ret;

    double destMs, currentMs;
    if (audio_ == nullptr) {
        return;
    }
    // 初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
    avformat_network_init();
    AVFormatContext *pFmtCtx = nullptr;
    ret = avformat_open_input(&pFmtCtx, this->videoPath.toLocal8Bit().data(), NULL,
                              NULL);  // 打开音视频文件并创建AVFormatContext结构体以及初始化.
    if (ret != 0) {
        return;
    }
    ret = avformat_find_stream_info(pFmtCtx, NULL);  // 初始化流信息

    if (ret != 0) {
        return;
    }

    int audioindex = -1;

    audioindex = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    AVCodec *acodec = avcodec_find_decoder(pFmtCtx->streams[audioindex]->codecpar->codec_id);  // 获取codec

    AVCodecContext *acodecCtx = avcodec_alloc_context3(acodec);  // 构造AVCodecContext ,并将vcodec填入AVCodecContext中
    avcodec_parameters_to_context(acodecCtx, pFmtCtx->streams[audioindex]->codecpar);  // 初始化AVCodecContext

    ret = avcodec_open2(
            acodecCtx, NULL,
            NULL);  // 打开解码器,由于之前调用avcodec_alloc_context3(vcodec)初始化了vc,那么codec(第2个参数)可以填NULL
    if (ret != 0) {
        return;
    }
    SwrContext *swrctx = nullptr;
    int sampleRate = acodecCtx->sample_rate > 44100 ? acodecCtx->sample_rate : 44100;
    swrctx = swr_alloc_set_opts(swrctx, av_get_default_channel_layout(2), AV_SAMPLE_FMT_S16, sampleRate,
                                acodecCtx->channel_layout, acodecCtx->sample_fmt, acodecCtx->sample_rate, NULL, NULL);
    swr_init(swrctx);

    double ddd = pFmtCtx->duration* 1000;
//    Q_EMIT duration(ddd);
    destMs = av_q2d(pFmtCtx->streams[audioindex]->time_base) * 1000 * pFmtCtx->streams[audioindex]->duration;

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    audio_->stop();
    QIODevice *io = audio_->start();

    while (true) {

//        if (type_ == kControl_Seek || seekMs_ > 0) {
//            auto value = (seekMs_ * destMs / 1000) * AV_TIME_BASE + static_cast<double>(pFmtCtx->start_time);
//            av_seek_frame(pFmtCtx, -1, value, AVSEEK_FLAG_BACKWARD);
//            type_ = kControl_None;
//            seekMs_ = -1;
//            Q_EMIT seekOk();
//        }

        ret = av_read_frame(pFmtCtx, packet);

        if (ret != 0) {
            if (ret == AVERROR_EOF) {
                break;
            }
            continue;
        }
        if (packet->stream_index == audioindex) {
            // 解码一帧数据
            ret = avcodec_send_packet(acodecCtx, packet);
            av_packet_unref(packet);

            if (ret != 0) {
                continue;
            }

            while (avcodec_receive_frame(acodecCtx, frame) == 0) {
                uint8_t *data[2] = {0};
                int byteCnt = frame->nb_samples * 2 * 2;

                unsigned char *pcm = new uint8_t[byteCnt];  // frame->nb_samples*2*2表示分配样本数据量*两通道*每通道2字节大小

                data[0] = pcm;  // 输出格式为AV_SAMPLE_FMT_S16(packet类型),所以转换后的LR两通道都存在data[0]中

                ret = swr_convert(swrctx, data, frame->nb_samples,                    // 输出
                                  (const uint8_t **)frame->data, frame->nb_samples);  // 输入

                // 将重采样后的data数据发送到输出设备,进行播放
                while (audio_->bytesFree() < byteCnt) {
                    msleep(10);
                }

                io->write((const char *)pcm, byteCnt);

                currentMs = av_q2d(pFmtCtx->streams[audioindex]->time_base) * 1000 * frame->pts;

                delete[] pcm;
            }
        }
    }

    // 释放内存
    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swrctx);
    avcodec_free_context(&acodecCtx);
    avformat_close_input(&pFmtCtx);
}

bool FfmpegThread::initAudio(int SampleRate) {
    QAudioFormat format;

    if (audio_ != nullptr) return true;

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    // Set up the format, eg.
    format = info.preferredFormat();
    format.setCodec("audio/pcm");
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    format.setChannelCount(2);
#else
    format.setChannels(2);
#endif
    format.setSampleRate(SampleRate);
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    if (!info.isFormatSupported(format)) {
        qDebug() << (tr("Audio format not supported by backend. Trying nearest format."));
        format = info.nearestFormat(format);
    }

    if (!info.isFormatSupported(format)) {
        qDebug() << "not support format";
        return false;
    }

    audio_ = std::make_shared<QAudioOutput>(format);

    audio_->setBufferSize(100000);

    return true;
}