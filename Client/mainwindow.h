#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTimer>

class QTcpSocket;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QSharedPointer<QTcpSocket> socket, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void sendSearchRequest();

private:
    Ui::MainWindow *ui;
    QSharedPointer<QTcpSocket> m_socket;
    QTimer *m_timer;
};
#endif // MAINWINDOW_H
