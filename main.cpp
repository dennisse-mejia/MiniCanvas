#include "mainwindow.h"
#include "loginwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow w;
    w.show();
    return a.exec();
}




