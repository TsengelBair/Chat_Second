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

    connect(ui->pushButton, &QPushButton::clicked, this, &AuthForm::sendAuthRequest);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AuthForm::sendAuthRequest);

    connect(this, &AuthForm::signalSendAuthRequest, m_networkManager.data(), &NetworkManager::slotSendPacket);
}

AuthForm::~AuthForm()
{
    delete ui;
}

void AuthForm::sendAuthRequest()
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
