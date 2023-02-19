#pragma once

#include <QtQuick/QQuickFramebufferObject>
#include <QJsonObject>
#include "FBORender.h"
#include "FFmpegThread.h"

//QML UI 相关逻辑放到 QQuickFramebufferObject 子类
//渲染相关放到 QQuickFramebufferObject::Renderer 子类
//该类仅在 Qt Quick 通过 OpenGL 渲染时才起作用
class FBORender;
class FBOItem : public QQuickFramebufferObject
{
Q_OBJECT
public:
    typedef std::function<void(int texture)> CallbackFun;
    FBOItem(QQuickItem *parent = nullptr);
    Q_PROPERTY(QJsonObject json READ json WRITE setJson)
    Q_PROPERTY(QString fragment READ fragment WRITE setFragment)
    Q_PROPERTY(QString vertex READ vertex WRITE setVertex)
    Q_INVOKABLE int getTextureID();
    Q_SIGNAL void updateTextureID(int textureID) const;
    Q_SLOT void renderNextFrame(uchar *bits, int width, int height);
    //Renderer 实例是从 createRenderer() 返回的
    QQuickFramebufferObject::Renderer *createRenderer() const override;
    QJsonObject json() { return m_json; }
    void setJson(QJsonObject json);
    void updateTexture(int texture);

    QString fragment() { return m_fragment; }
    void setFragment(QString fragment) { m_fragment = fragment; }

    QString vertex() { return m_vertex; }
    void setVertex(QString vertex) { m_vertex = vertex; }
    CallbackFun callbackFun() { return m_callfuct; }
private:
    QJsonObject m_json;
    QString m_fragment;
    QString m_vertex;
    int textureID;
    CallbackFun m_callfuct;
    FfmpegThread* ffmpegThread;
    mutable FBORender* render;
};
