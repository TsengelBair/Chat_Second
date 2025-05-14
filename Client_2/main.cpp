#include <QApplication>
#include <QSharedPointer>

#include "authform.h"
#include "networkmanager.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /// сначала создаем экземпляр сетевого менеджера
    QSharedPointer<NetworkManager> networkManager(new NetworkManager());
    if (!networkManager->connectToServer("127.0.0.1", 5000)) {
        qDebug() << "Ошибка, не удалось  подключиться к серверу";
    } else {
        qDebug() << "Успешное подключение к серверу";
    }

    /// создаем форму авторизации передавая ранее созданного сетевого менеджера
    QSharedPointer<AuthForm> authForm(new AuthForm(networkManager));
    authForm->show();

    QObject::connect(networkManager.data(), &NetworkManager::signalAuthResponseReceived,
             authForm.data(), &AuthForm::slotAuthResponseReceived, Qt::UniqueConnection);

    return a.exec();
}
