#include "app/mainwindow.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/calib_tools.png"));
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
