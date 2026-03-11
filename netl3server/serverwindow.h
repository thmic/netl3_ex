#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QList>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void onProtocolChanged(int index);
    void onStartClicked();
    void onSendClicked();
    void onNewTcpConnection();
    void onTcpDataReceived();
    void onUdpDataReceived();
    void onTcpClientDisconnected();

private:
    void setupUI();
    void setupConnections();
    void appendMessage(const QString &type, const QString &content, const QString &color = "");
    void startTcpServer();
    void stopTcpServer();
    void startUdpServer();
    void stopUdpServer();
    void clearAll();

    // UI控件
    QComboBox *m_protocolCombo;
    QSpinBox *m_portSpinBox;
    QPushButton *m_startBtn;
    QTextBrowser *m_msgDisplay;
    QLineEdit *m_msgInput;
    QPushButton *m_sendBtn;
    QLabel *m_statusLabel;

    // 网络对象
    QTcpServer *m_tcpServer;
    QList<QTcpSocket*> m_tcpClients;
    QUdpSocket *m_udpSocket;
    
    // 状态变量
    bool m_isRunning;
    QString m_lastUdpAddress;
    quint16 m_lastUdpPort;
};

#endif // SERVERWINDOW_H
