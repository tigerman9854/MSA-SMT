#include "UserInterface.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    UserInterface w;
    w.show();
    return a.exec();
}
