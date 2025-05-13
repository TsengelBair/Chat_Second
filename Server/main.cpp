#include <QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server(QHostAddress::LocalHost, 5000);

    return a.exec();
}
