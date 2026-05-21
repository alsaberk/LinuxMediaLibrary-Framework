#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // --- YAZILIMSAL ZIRH: Donanım ve Sürücü Uyumluluğu ---
    
    // Qt RHI Katmanını OpenGL'e sabitle (AMD/Wayland/X11 uyumluluğu için)
    qputenv("QSG_RHI_BACKEND", "opengl");
    
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    
    qputenv("QSG_RHI_BACKEND", "opengl");
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");

    // Kaynak paylaşımını aç (Threadler arası GL context paylaşımı)
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    int result = a.exec();
    
    return result;
}
