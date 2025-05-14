#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "networkmanager.h"
#include "serializer.h"

MainWindow::MainWindow(QSharedPointer<NetworkManager> networkManager, int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_networkManager(networkManager),
    m_userId(userId)
{
    ui->setupUi(this);

    connect(this, &MainWindow::signalSendGetDefaultDataRequest, m_networkManager.data(),
                                 &NetworkManager::slotSendPacket, Qt::UniqueConnection);

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

void MainWindow::slotGetDefaultDataResponseReceived(const QByteArray &data, const ResponseType &responseType)
{

}
