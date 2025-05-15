#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "networkmanager.h"
#include "serializer.h"
#include "../ProtoFiles/IGetResponse.pb.h"

MainWindow::MainWindow(QSharedPointer<NetworkManager> networkManager, int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_networkManager(networkManager),
    m_userId(userId)
{
    ui->setupUi(this);

    connect(this, &MainWindow::signalSendGetDefaultDataRequest, m_networkManager.data(),
                                 &NetworkManager::slotSendPacket, Qt::UniqueConnection);

    connect(networkManager.data(), &NetworkManager::signalGetDefaultDataResponseReceived,
                                  this, &MainWindow::slotGetDefaultDataResponseReceived);

    /// после успешной авторизации отправляем get запрос на получение данных, которые подругрузим в ui (чаты и тд)
    createGetDefaultDataRequest();

    this->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createGetDefaultDataRequest()
{
    RequestType requestType = RequestType::REQUEST_GET_DEFAULT_DATA;
    QByteArray data = Serializer::serializeGetReq(m_userId);

    emit signalSendGetDefaultDataRequest(data, requestType);
}

void MainWindow::slotGetDefaultDataResponseReceived(const QByteArray &data)
{
    /// вынести в класс Serializer
    IGetResponse getResponse;

    if (!getResponse.ParseFromArray(data.data(), data.size())) {
        qWarning() << "Failed to parse response data.";
        return;
    }

    if (getResponse.is_empty()) {
        ui->stackedWidget->setCurrentIndex(1);
        return;
    }

    /// добавить подгрузку (когда будут данные)
}
