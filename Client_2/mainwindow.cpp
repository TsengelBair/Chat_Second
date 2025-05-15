#include <QListWidget>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "networkmanager.h"
#include "serializer.h"
#include "types.h"

MainWindow::MainWindow(QSharedPointer<NetworkManager> networkManager, int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_networkManager(networkManager),
    m_userId(userId)
{
    ui->setupUi(this);

    connect(this, &MainWindow::signalSendGetDefaultDataRequest, m_networkManager.data(),
                                 &NetworkManager::slotSendPacket, Qt::UniqueConnection);

    connect(this, &MainWindow::singalSendSearchUsersRequest, m_networkManager.data(),
                                 &NetworkManager::slotSendPacket, Qt::UniqueConnection);

    connect(networkManager.data(), &NetworkManager::signalGetDefaultDataResponseReceived,
            this, &MainWindow::slotGetDefaultDataResponseReceived, Qt::UniqueConnection);

    connect(networkManager.data(), &NetworkManager::signalGetFoundedUsersResponseReceived,
            this, &MainWindow::slotSearchUsersResponseReceived, Qt::UniqueConnection);

    connect(ui->chatsLW, &QListWidget::itemClicked, this, &MainWindow::slotChatSelected);

    /// после успешной авторизации отправляем get запрос на получение данных, которые подругрузим в ui (чаты и тд)
    sendGetDefaultDataRequest();

    /// инициализация таймера, для логики поисковой строки
    initializeTimer();

    this->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeTimer()
{
    m_timer = new QTimer(this);

    m_timer->setInterval(500); /// запрос поиска на сервер отправляется с задержкой пол секунды
    m_timer->setSingleShot(true); /// чтобы таймер срабатывал единожды и автоматически останавливался

    connect(ui->searchLE, &QLineEdit::textChanged, this, [&](){ m_timer->start(); }, Qt::UniqueConnection);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::sendSearchUsersRequest, Qt::UniqueConnection);
}

void MainWindow::sendGetDefaultDataRequest()
{
    RequestType requestType = RequestType::REQUEST_GET_DEFAULT_DATA;
    QByteArray data = Serializer::serializeGetReq(m_userId);

    emit signalSendGetDefaultDataRequest(data, requestType);
}

void MainWindow::sendSearchUsersRequest()
{
    RequestType requestType = RequestType::REQUEST_FIND_USERS;

    QString loginToSearch = ui->searchLE->text();
    if (loginToSearch.isEmpty()) return;

    QByteArray data = Serializer::serializeSearchUsersReq(loginToSearch);
    emit singalSendSearchUsersRequest(data, requestType);
}

void MainWindow::slotChatSelected(QListWidgetItem *item)
{
    ui->stackedWidget->setCurrentIndex(0);
    /// Установить заголовок чата
    ui->chatHeaderUserNameLB->setText(item->text());

    /// Получить userId из данных элемента
    int selectedUserId = item->data(Qt::UserRole).toInt();

    /// Выводим userId в консоль
    qDebug() << "Выбранный userId:" << selectedUserId;

    /// Подгрузить сообщения
    /// Для этого необходимо написать прото файл с запросом на получение всех сообщений с выбранным пользователем
}

void MainWindow::slotGetDefaultDataResponseReceived(const QByteArray &data)
{
    QVector<Chat> chats = Serializer::deserializeGetDefaultDataResponse(data);
    if (chats.empty()) {
        ui->stackedWidget->setCurrentIndex(1);
    }

    /// добавить отображение чатов при непустом ответе
}

void MainWindow::slotSearchUsersResponseReceived(const QByteArray &data)
{
    /// очистка предыдущих значений
    ui->chatsLW->clear();

    QList<QPair<int, QString>> foundUsers = Serializer::deserializeFoundUsersResponse(data);

    if (foundUsers.empty()) {
        qDebug() << "Пользователи с похожим логином не найдены";
        return;
    }

    for (const auto &user : foundUsers) {
        QListWidgetItem *item = new QListWidgetItem(user.second);
        item->setData(Qt::UserRole, user.first);
        ui->chatsLW->addItem(item);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        ui->stackedWidget->setCurrentIndex(1);
    }

    QWidget::keyPressEvent(event);
}



