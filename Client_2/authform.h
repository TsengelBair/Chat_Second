#ifndef AUTHFORM_H
#define AUTHFORM_H

#include <QWidget>
#include <QSharedPointer>

#include "types.h"

class NetworkManager;

namespace Ui {
class AuthForm;
}

class AuthForm : public QWidget
{
    Q_OBJECT

public:
    explicit AuthForm(QSharedPointer<NetworkManager> networkManager, QWidget *parent = nullptr);
    ~AuthForm();

public slots:
    void slotAuthResponseReceived(const QByteArray &data, const ResponseType &responseType);

/// как понимаю, передача параметров по ссылке в сигналы не рекомендуется (компилятор все равно попытается скопировать)
/// потом протестить через дебаг и анализатор
signals:
    void signalSendAuthRequest(QByteArray &data, RequestType &requestType);

private slots:
    void sendAuthRequest();

private:
    Ui::AuthForm *ui;
    QSharedPointer<NetworkManager> m_networkManager;
};

#endif // AUTHFORM_H
