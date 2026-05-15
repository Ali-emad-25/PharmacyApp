#include "mainwindow.h"
#include "database.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file("D:/Project_C++/PharmacyApp/style.qss");
    if(file.open(QFile::ReadOnly)) {
        a.setStyleSheet(file.readAll());
    }

    if (!Database::init()) {
        return -1;
    }

    MainWindow w;
    w.showMaximized();

    return a.exec();
}