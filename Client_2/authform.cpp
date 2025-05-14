#include <QMessageBox>

#include "authform.h"
#include "ui_authform.h"
#include "networkmanager.h"
#include "serializer.h"
#include "mainwindow.h"

AuthForm::AuthForm(QSharedPointer<NetworkManager> networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthForm),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &AuthForm::createAuthRequest, Qt::UniqueConnection);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AuthForm::createAuthRequest, Qt::UniqueConnection);

    connect(this, &AuthForm::signalSendAuthRequest, m_networkManager.data(), &NetworkManager::slotSendPacket, Qt::UniqueConnection);
}

AuthForm::~AuthForm()
{
    delete ui;
}

void AuthForm::slotAuthResponseReceived(const QByteArray &data, const ResponseType &responseType)
{
    QPair<int, int> serverResponse = Serializer::deserializeAuthResponse(data);

    int serverResponseCode = serverResponse.first;
    int userId = serverResponse.second;

    if (serverResponseCode == 409) {
        QMessageBox::warning(this, "Предупреждение", "Registration failed: user already exists");
    } else if (serverResponseCode == 200) {
        /// создать объект MainWindow, который в конструкоре вып-т get запрос и там же вызовет метод show
        /// также необходимо передать id пользователя
        if (!mainWindow) {
            mainWindow.reset(new MainWindow(m_networkManager, userId));
            emit signalSuccessAuth();
        }
        return;
    } else if (serverResponseCode == 500) {
        QMessageBox::warning(this, "Предупреждение", "Registration failed: server error");
    } else if (serverResponseCode == 404) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: user not found");
    } else if (serverResponseCode == 403) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: invalid password");
    } else if (serverResponseCode == 201) {
        if (!mainWindow) {
            mainWindow.reset(new MainWindow(m_networkManager, userId));
            emit signalSuccessAuth();
        }
        return;
    }
}

void AuthForm::createAuthRequest()
{
    RequestType requestType;

    QString requestTypeStr = qobject_cast<QPushButton*>(sender())->text();

    if (requestTypeStr == "Login") requestType = RequestType::REQUEST_LOGIN;
    else requestType = RequestType::REQUEST_REGISTER;

    QString login = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    QByteArray data = Serializer::serializeAuthReq(login, password);
    emit signalSendAuthRequest(data, requestType);
}
