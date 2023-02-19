#include "FBOItem.h"

FBOItem::FBOItem(QQuickItem *parent)
        : QQuickFramebufferObject(parent)
{
    //上下翻转，这样就和OpenGL的坐标系一致了
    setMirrorVertically(true);
    m_callfuct = std::bind(&FBOItem::updateTexture, this, std::placeholders::_1);
    ffmpegThread = new FfmpegThread();
    connect(ffmpegThread, &FfmpegThread::nextFrames, this, &FBOItem::renderNextFrame, Qt::QueuedConnection);
    ffmpegThread->setPath("/Users/tiantian/Downloads/green.mov");
    ffmpegThread->start();
}

QQuickFramebufferObject::Renderer *FBOItem::createRenderer() const
{
    //Renderer 和 FBO 都是内部管理的内存
    Shader shader = {
            m_fragment,
            m_vertex
    };
    render = new FBORender(this->size().toSize(), shader);
    return render;
}

void FBOItem::setJson(QJsonObject json) {
    m_json = json;
    this->update();
}

int FBOItem::getTextureID() {
    return textureID;
}

void FBOItem::updateTexture(int textureID) {
    textureID = textureID;
    emit updateTextureID(textureID);
}

void FBOItem::renderNextFrame(uchar *bits, int width, int height) {
    if (render != nullptr) {
        render->imageBit = bits;
        render->imageWidth = width;
        render->imageHeight = height;
        this->update();
    }
}
