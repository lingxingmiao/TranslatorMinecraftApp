#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TranslatorMinecraft");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("TranslatorMC");

    MainWindow window;
    window.setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    window.setAttribute(Qt::WA_TranslucentBackground);
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
