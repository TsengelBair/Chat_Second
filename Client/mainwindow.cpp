#include <QTcpSocket>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "serializer.h"
#include "packetbuilder.h"
#include "types.h"

MainWindow::MainWindow(QSharedPointer<QTcpSocket> socket, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socket(std::move(socket))
{
    ui->setupUi(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(500); /// запрос поиска на сервер отправляется с задержкой пол секунды
    m_timer->setSingleShot(true); /// чтобы таймер срабатывал единожды и автоматически останавливался

    connect(ui->searchLE, &QLineEdit::textChanged, this, [&](){ m_timer->start(); });
    connect(m_timer, &QTimer::timeout, this, &MainWindow::sendSearchRequest);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendSearchRequest()
{
    QString loginToSearch = ui->searchLE->text();
    if (loginToSearch.isEmpty()) return;

    QByteArray data = Serializer::serializeSearchReq(loginToSearch);
    QByteArray packet = PacketBuilder::createPacketToSend(data, RequestType::FIND_USERS);

    m_socket.data()->write(packet);
}

