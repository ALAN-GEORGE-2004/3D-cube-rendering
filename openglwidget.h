#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>

struct Vertex
{
    QVector3D position;
    QVector3D normal;
    QVector3D color;
};

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    public:
        explicit OpenGLWidget(QWidget *parent = nullptr);
        ~OpenGLWidget();

    protected:
        void initializeGL() override;
        void resizeGL(int width,int height) override;
        void paintGL() override;

        // Mouse events
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;


    private:
        QOpenGLBuffer m_vbo;
        QOpenGLVertexArrayObject m_vao;
        QOpenGLShaderProgram *m_shaderProgram;
        QMatrix4x4 m_projection;

        float m_aspectRatio;

        // Mouse-controlled rotation
        float m_rotationX;
        float m_rotationY;
        QPoint m_lastMousePosition;
        bool m_mousePressed;
        float m_zoom;
    };

#endif