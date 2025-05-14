#ifndef AUTHFORM_H
#define AUTHFORM_H

#include <QWidget>
#include <QSharedPointer>

#include "types.h"

class NetworkManager;
class MainWindow;

namespace Ui {
class AuthForm;
}

class AuthForm : public QWidget
{
    Q_OBJECT

public:
    explicit AuthForm(QSharedPointer<NetworkManager> networkManager, QWidget *parent = nullptr);
    ~AuthForm();

signals:
    void signalSendAuthRequest(QByteArray &data, RequestType &requestType);
    void signalSuccessAuth();

public slots:
    void slotAuthResponseReceived(const QByteArray &data, const ResponseType &responseType);

private slots:
    void createAuthRequest();

private:
    Ui::AuthForm *ui;
    QSharedPointer<NetworkManager> m_networkManager;
    QSharedPointer<MainWindow> mainWindow;
};

#endif // AUTHFORM_H
