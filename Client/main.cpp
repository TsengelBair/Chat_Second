#include "authform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AuthForm form;
    form.show();

    return a.exec();
}
