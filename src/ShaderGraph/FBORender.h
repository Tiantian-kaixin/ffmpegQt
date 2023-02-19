#pragma once

#include <QtQuick/QQuickFramebufferObject>
#include <QtGui/QOpenGLFramebufferObject>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions_3_3_Core>
#include <QJsonObject>
#include <QJsonArray>
#include "FBOItem.h"
#include "qimage.h"
#include <QOpenGLTexture>

struct Shader {
    QString fragment;
    QString vertex;
};
class FBORender : public QQuickFramebufferObject::Renderer,
                  protected QOpenGLFunctions_3_3_Core
{
public:
    FBORender(QSize size, Shader shader);
    ~FBORender();
    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void synchronize(QQuickFramebufferObject *item) override;
    void doRender();
    Q_SLOT void renderNextFrame(uchar *bits, int width, int height);
    uchar* imageBit;
    int imageWidth;
    int imageHeight;

protected:
    void doInitialize();
    void doFree();
    //着色器程序
    QOpenGLShaderProgram program;
    Shader m_shader;
    QJsonObject m_json;
    std::function<void(int texture)> m_callback;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

private:
    QSize m_size;
};
