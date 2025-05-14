#include <QMessageBox>

#include "authform.h"
#include "ui_authform.h"
#include "networkmanager.h"
#include "serializer.h"

AuthForm::AuthForm(QSharedPointer<NetworkManager> networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthForm),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &AuthForm::createAuthRequest, Qt::UniqueConnection);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AuthForm::createAuthRequest, Qt::UniqueConnection);

    connect(this, &AuthForm::signalSendAuthRequest, m_networkManager.data(), &NetworkManager::slotSendPacket, Qt::UniqueConnection);
    connect(this, &AuthForm::signalSendGetDefaultDataRequest, m_networkManager.data(),
                               &NetworkManager::slotSendPacket, Qt::UniqueConnection);
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
        createGetDefaultDataRequest(userId);
        return;
    } else if (serverResponseCode == 500) {
        QMessageBox::warning(this, "Предупреждение", "Registration failed: server error");
    } else if (serverResponseCode == 404) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: user not found");
    } else if (serverResponseCode == 403) {
        QMessageBox::warning(this, "Предупреждение", "Login failed: invalid password");
    } else if (serverResponseCode == 201) {
        createGetDefaultDataRequest(userId);
        return;
    }
}

void AuthForm::slotGetDefaultDataResponseReceived(const QByteArray &data)
{
    /// создать MainWindow передав туда десерилизованные данные
    /// для представления данных необходимо предварительно создать структуру для представления этих данных
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

void AuthForm::createGetDefaultDataRequest(const int userId)
{
    RequestType requestType = RequestType::REQUEST_GET_DEFAULT_DATA;
    QByteArray data = Serializer::serializeGetReq(userId);

    emit signalSendGetDefaultDataRequest(data, requestType);
}
