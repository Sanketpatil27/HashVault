#include "HashVault.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HashVault window;
    window.show();
    return app.exec();
}
