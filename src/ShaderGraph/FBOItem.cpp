#include "FBOItem.h"

FBOItem::FBOItem(QQuickItem *parent)
        : QQuickFramebufferObject(parent)
{
    //上下翻转，这样就和OpenGL的坐标系一致了
    setMirrorVertically(true);
    m_callfuct = std::bind(&FBOItem::updateTexture, this, std::placeholders::_1);
}

QQuickFramebufferObject::Renderer *FBOItem::createRenderer() const
{
    //Renderer 和 FBO 都是内部管理的内存
    Shader shader = {
            m_fragment,
            m_vertex
    };
    return new FBORender(this->size().toSize(), shader);
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
