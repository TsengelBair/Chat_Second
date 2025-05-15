#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class NetworkManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSharedPointer<NetworkManager> networkManager, int userId, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signalSendGetDefaultDataRequest(QByteArray &data, RequestType &requestType);
    void singalSendSearchUsersRequest(QByteArray &data, RequestType &requestType);

public slots:
    void slotGetDefaultDataResponseReceived(const QByteArray &data);
    void slotSearchUsersResponseReceived(const QByteArray &data);

private slots:
    void sendGetDefaultDataRequest();
    void sendSearchUsersRequest();

private:
    void initializeTimer();

private:
    Ui::MainWindow *ui;
    QSharedPointer<NetworkManager> m_networkManager;
    int m_userId;
    QTimer* m_timer;
};
#endif // MAINWINDOW_H
