#ifndef AUTHFORM_H
#define AUTHFORM_H

#include <QWidget>
#include <QTcpSocket>
#include <QScopedPointer>

#include "types.h"

namespace Ui {
class AuthForm;
}

class MainWindow;

class AuthForm : public QWidget
{
    Q_OBJECT

public:
    explicit AuthForm(QWidget *parent = nullptr);
    ~AuthForm();

private slots:
    void handleIncommingDataFromServer();
    void handleAuthResponse(const QByteArray &data, const ResponseType &responseType);
    void sendAuthReq();
    void sendGetReq(const int userId);

private:
    void connectToServer();

private:
    Ui::AuthForm *ui;
    QTcpSocket* m_socket;
    QScopedPointer<MainWindow> mainWindow;
};

#endif // AUTHFORM_H
