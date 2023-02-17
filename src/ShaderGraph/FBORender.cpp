#include "FBORender.h"

#include <QDebug>

FBORender::FBORender(QSize size, Shader shader): m_shader(shader), m_json({})
{
    doInitialize();
}

FBORender::~FBORender()
{
    doFree();
}

void FBORender::render()
{
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //渲染
    doRender();
}

QOpenGLFramebufferObject *FBORender::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    return new QOpenGLFramebufferObject(size, format);
}

void FBORender::synchronize(QQuickFramebufferObject *item)
{
    FBOItem *fbo = static_cast<FBOItem*>(item);
    m_json = fbo->json();
    m_callback = fbo->callbackFun();
    if (this->framebufferObject() != nullptr) {
        qDebug() << "framebufferObject: " << this->framebufferObject()->texture();
        fbo->updateTexture(this->framebufferObject()->texture());
    }
}

void FBORender::doInitialize()
{
    //为当前上下文初始化OpenGL函数解析
    initializeOpenGLFunctions();
    //将source编译为指定类型的着色器，并添加到此着色器程序
    if(!program.addCacheableShaderFromSourceFile(
            QOpenGLShader::Vertex, m_shader.vertex)){
        qDebug()<<"compiler vertex error";
        exit(0);
    }
    if(!program.addCacheableShaderFromSourceFile(
            QOpenGLShader::Fragment, m_shader.fragment)){
        qDebug()<<"compiler fragment error";
        exit(0);
    }
    //使用addShader()将添加到该程序的着色器链接在一起。
    program.link();

    //将属性名称绑定到指定位置(这里location=0)
    //program.bindAttributeLocation("vertices", 0);
    float vertices[] = {
            1.0f,  1.0f, 0.0f,  // top right
            1.0f, -1.0f, 0.0f,  // bottom right
            -1.0f, -1.0f, 0.0f,  // bottom left
            -1.0f,  1.0f, 0.0f   // top left
    };
    unsigned int indices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    //unbind
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void FBORender::doFree()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void FBORender::doRender() {
    program.bind();
    QJsonObject::iterator it;
    for (it = m_json.begin(); it != m_json.end(); it++) {
        if (it.key() == "uniform") {
            QJsonObject uniformJson = m_json["uniform"].toObject();
            QJsonObject::iterator it_child;
            for (it_child = uniformJson.begin(); it_child != uniformJson.end(); it_child++) {
                QJsonValueRef value = it_child.value();
                if (value.isBool()) {
                    program.setUniformValue(it_child.key().toStdString().c_str(), value.toBool());
                } else if (value.isArray()) {
                    QJsonArray array = value.toArray();
                    if (array.size() == 2) {
                        auto vec2 = QVector2D(array[0].toDouble(), array[1].toDouble());
                        program.setUniformValue(it_child.key().toStdString().c_str(), vec2);
                    } else if (array.size() == 3) {
                        auto vec3 = QVector3D(array[0].toDouble(), array[1].toDouble(), array[2].toDouble());
                        program.setUniformValue(it_child.key().toStdString().c_str(), vec3);
                    } else if (array.size() == 4) {
                        auto vec4 = QVector4D(array[0].toDouble(), array[1].toDouble(), array[2].toDouble(), array[3].toDouble());
                        program.setUniformValue(it_child.key().toStdString().c_str(), vec4);
                    }
                } else if (value.isDouble()) {
                    program.setUniformValue(it_child.key().toStdString().c_str(), static_cast<GLfloat>(value.toDouble()));
                }
            }
        } else if (it.key() == "texture") {
            QJsonObject uniformJson = m_json["texture"].toObject();
            QJsonObject::iterator it_child;
            for (it_child = uniformJson.begin(); it_child != uniformJson.end(); it_child++) {
                QJsonValueRef value = it_child.value();
                if (value.isDouble()) {
                    float textureID = static_cast<float>(value.toDouble());
                    if (textureID <= 0) {
                        continue;
                    }
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, (GLuint)textureID);
                    program.setUniformValue(it_child.key().toStdString().c_str(), 0);
                }
            }
        } else if (it.key() == "file") {
            QJsonObject uniformJson = m_json["file"].toObject();
            QJsonObject::iterator it_child;
            for (it_child = uniformJson.begin(); it_child != uniformJson.end(); it_child++) {
                QJsonValueRef value = it_child.value();
                if (value.isString()) {
                    QString filePath = value.toString();
                    auto* qImage = new QImage(filePath);
                    if (qImage->isNull()) {
                        continue;
                    }
                    // (一) glGenTexture
                    GLuint textureID = 0;
                    glGenTextures(1, &textureID);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textureID);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, qImage->width(), qImage->height(), 0, GL_BGRA,
                                 GL_UNSIGNED_BYTE, qImage->mirrored().bits());
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // (二) QOpenGLTexture
//                    auto* texture = new QOpenGLTexture(qImage->mirrored());
//                    GLuint textureID = texture->textureId();
//                    if (textureID <= 0) {
//                        continue;
//                    }
//                    glActiveTexture(GL_TEXTURE0);
//                    glBindTexture(GL_TEXTURE_2D, textureID);
                    program.setUniformValue(it_child.key().toStdString().c_str(), 0);
                }
            }
        }
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    program.release();
}
