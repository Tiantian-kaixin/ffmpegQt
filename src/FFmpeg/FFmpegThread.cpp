//
// Created by TIANTIAN on 2023/2/17.
//

#include "FFmpegThread.h"

FfmpegThread::FfmpegThread(QObject *parent) {

}

FfmpegThread::~FfmpegThread() {

}
void FfmpegThread::setPath(const QString &src) {
    audioPath = src;
}

int64_t FfmpegThread::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    return value.count();
}

void FfmpegThread::run() {
    AVFormatContext *pFormatCtx; // 存储音视频封装格式中包含的信息
    int videoIndex = -1; // 视频帧索引，初始化为-1
    AVCodecContext *pCodecCtx; // 视频流编码结构
    AVCodec *pCodec; // 视频解码器
    AVFrame *pFrame, *pFrameRGB;
    unsigned char *out_buffer;
    AVPacket *packet;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx; // 主要用于视频图像的转换

    // 注册FFMpeg的库
    av_register_all();

    /*** （一）打开音视频流并获取音视频流信息 ***/
    // 初始化AVFormatContext
    pFormatCtx = avformat_alloc_context();
    // 打开音视频流
    if (avformat_open_input(&pFormatCtx, audioPath.toLocal8Bit().data(), nullptr, nullptr) != 0)
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

    /*** （二）查找视频流位置以及查找并打开视频解码器 ***/
    // 查找视频流的起始索引位置（nb_streams表示视音频流的个数）
    for (int i = 0; i < (int)pFormatCtx->nb_streams; i++)
    {
        // 查找到视频流时退出循环
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) // 判断是否为视频流
        {
            videoIndex = i;
            break;
        }
    }
    if (videoIndex == -1)
    {
        printf("Didn't find a video stream.\n");
        return ;
    }
    // 查找视频解码器
    pCodecCtx = pFormatCtx->streams[videoIndex]->codec; // 获取视频流编码结构
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr)
    {
        printf("Codec not found.\n");
        return ;
    }
    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.\n");
        return ;
    }
    // 打印视频信息
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, audioPath.toLocal8Bit(), 0); // 此函数打印输入或输出的详细信息
    printf("-------------------------------------------------\n");

    /*** （三）视频解码的同时处理图片像素数据 ***/
    // 创建帧结构，此函数仅分配基本结构空间，图像数据空间需通过av_malloc分配
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    // 创建动态内存,创建存储图像数据的空间（av_image_get_buffer_size获取一帧图像需要的大小）
    out_buffer = (unsigned char *)av_malloc((size_t)av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1));
    // 存储一帧像素数据缓冲区
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
                         AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    // 初始化img_convert_ctx结构
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);
    // av_read_frame读取一帧未解码的数据
    int64_t start_time = getCurrentTime();
    while (av_read_frame(pFormatCtx, packet) >= 0)
    {
        // 如果是视频数据
        if (packet->stream_index == videoIndex)
        {
            // 解码一帧视频数据
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0)
            {
                printf("Decode Error.\n");
                return ;
            }
            if (got_picture)
            {
                sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);
                emit nextFrames((uchar*)pFrameRGB->data[0], pCodecCtx->width,pCodecCtx->height);
                AVRational time_base = pFormatCtx->streams[videoIndex]->time_base;
                AVRational time_base_q = {1, AV_TIME_BASE};
                int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);
                int64_t now_time = (getCurrentTime() - start_time) * 1000;
                if (pts_time > now_time) {
                    QThread::usleep(pts_time - now_time);
                }
            }
        }
        av_free_packet(packet);
    }

    /*** （四）最后要释放申请的内存空间 ***/
    sws_freeContext(img_convert_ctx); // 释放一个SwsContext
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}