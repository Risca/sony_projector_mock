#include <QCoreApplication>

#include "serialmock.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SerialMock mock(&a);

    return a.exec();
}
