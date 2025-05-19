#include <QApplication>
#include <QPushButton>
#include "HomeWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    HomeWindow window;
    window.show();
    return app.exec();
}