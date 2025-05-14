#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "networkmanager.h"

MainWindow::MainWindow(QSharedPointer<NetworkManager> networkManager, int userId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_networkManager(networkManager),
    m_userId(userId)
{
    ui->setupUi(this);
    this->show();
    /// выполнить get запрос по m_userId
}

MainWindow::~MainWindow()
{
    delete ui;
}

