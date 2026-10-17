#include <QApplication>
#include <QSurfaceFormat>

#include "openglwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //for solving the issue of jaggered line
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    // Enable 4x MSAA
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    OpenGLWidget window;//Notice that here we are creating the window of OpenGLWidget.
    window.resize(800, 600);//for setting the size of window while application opens
    window.show();
    return app.exec();
}