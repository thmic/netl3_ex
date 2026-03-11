#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void onProtocolChanged(int index);
    void onConnectClicked();
    void onSendClicked();
    void onTcpConnected();
    void onTcpDisconnected();
    void onDataReceived();

private:
    void setupUI();
    void setupConnections();
    void appendMessage(const QString &type, const QString &content, const QString &color = "");
    void connectToServer();
    void disconnectFromServer();
    void startUdpClient();
    void stopUdpClient();
    void clearAll();

    // UI控件
    QComboBox *m_protocolCombo;
    QLineEdit *m_addressInput;
    QSpinBox *m_portSpinBox;
    QPushButton *m_connectBtn;
    QTextBrowser *m_msgDisplay;
    QLineEdit *m_msgInput;
    QPushButton *m_sendBtn;
    QLabel *m_statusLabel;

    // 网络对象
    QTcpSocket *m_tcpSocket;
    QUdpSocket *m_udpSocket;
    
    // 状态变量
    bool m_isConnected;
    QString m_targetAddress;
    quint16 m_targetPort;
};

#endif // CLIENTWINDOW_H
