#include "serverwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QHostAddress>
#include <QTextCursor>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tcpServer(nullptr)
    , m_udpSocket(nullptr)
    , m_isRunning(false)
    , m_lastUdpPort(0)
{
    setupUI();
    setupConnections();
    clearAll();
}

ServerWindow::~ServerWindow()
{
    if (m_isRunning) {
        if (m_tcpServer && m_tcpServer->isListening()) {
            stopTcpServer();
        }
        if (m_udpSocket && m_udpSocket->isOpen()) {
            stopUdpServer();
        }
    }
    
    // 清理TCP客户端
    qDeleteAll(m_tcpClients);
    m_tcpClients.clear();
    
    delete m_tcpServer;
    delete m_udpSocket;
}

void ServerWindow::setupUI()
{
    // 设置主窗口
    setWindowTitle("Qt Network Debugger - 服务端");
    resize(600, 500);
    
    // 创建中心widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 控制区
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    m_protocolCombo = new QComboBox();
    m_protocolCombo->addItem("TCP Server");
    m_protocolCombo->addItem("UDP Server");
    
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(3890);
    
    m_startBtn = new QPushButton("启动服务");
    
    controlLayout->addWidget(new QLabel("协议:"));
    controlLayout->addWidget(m_protocolCombo);
    controlLayout->addWidget(new QLabel("端口:"));
    controlLayout->addWidget(m_portSpinBox);
    controlLayout->addWidget(m_startBtn);
    controlLayout->addStretch();
    
    // 状态栏
    m_statusLabel = new QLabel("服务状态: 已停止");
    
    // 消息显示窗口
    m_msgDisplay = new QTextBrowser();
    m_msgDisplay->setReadOnly(true);
    
    // 输入区
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_msgInput = new QLineEdit();
    m_msgInput->setPlaceholderText("输入消息...");
    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setEnabled(false);
    
    inputLayout->addWidget(m_msgInput);
    inputLayout->addWidget(m_sendBtn);
    
    // 组装主布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_msgDisplay);
    mainLayout->addLayout(inputLayout);
}

void ServerWindow::setupConnections()
{
    connect(m_protocolCombo, &QComboBox::currentIndexChanged,
            this, &ServerWindow::onProtocolChanged);
    connect(m_startBtn, &QPushButton::clicked, this, &ServerWindow::onStartClicked);
    connect(m_sendBtn, &QPushButton::clicked, this, &ServerWindow::onSendClicked);
    connect(m_msgInput, &QLineEdit::returnPressed, this, &ServerWindow::onSendClicked);
}

void ServerWindow::onProtocolChanged(int index)
{
    if (m_isRunning) {
        QMessageBox::warning(this, "警告", "服务运行中，无法切换协议！");
        // 恢复之前的选项
        m_protocolCombo->setCurrentIndex(index == 0 ? 1 : 0);
        return;
    }
    
    QString protocol = (index == 0) ? "TCP" : "UDP";
    appendMessage("系统", QString("协议已切换至 %1").arg(protocol));
    clearAll();
}

void ServerWindow::onStartClicked()
{
    if (!m_isRunning) {
        // 启动服务
        int protocolIndex = m_protocolCombo->currentIndex();
        
        if (protocolIndex == 0) {
            // TCP Server
            startTcpServer();
        } else {
            // UDP Server
            startUdpServer();
        }
    } else {
        // 停止服务
        int protocolIndex = m_protocolCombo->currentIndex();
        if (protocolIndex == 0) {
            stopTcpServer();
        } else {
            stopUdpServer();
        }
    }
}

void ServerWindow::onSendClicked()
{
    QString message = m_msgInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    int protocolIndex = m_protocolCombo->currentIndex();
    if (protocolIndex == 0) {
        // TCP 发送
        if (m_tcpClients.isEmpty()) {
            appendMessage("错误", "无客户端连接");
            return;
        }
        
        // 发送给所有客户端（广播）
        for (QTcpSocket *client : m_tcpClients) {
            if (client->state() == QTcpSocket::ConnectedState) {
                client->write(message.toUtf8());
                QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
                appendMessage("发送", QString("%1 ◀ %2").arg(clientInfo).arg(message), "#800080");
            }
        }
    } else {
        // UDP 发送
        if (m_lastUdpPort == 0) {
            appendMessage("错误", "尚未收到任何UDP消息，无法确定发送目标");
            return;
        }
        
        m_udpSocket->writeDatagram(message.toUtf8(), QHostAddress(m_lastUdpAddress), m_lastUdpPort);
        appendMessage("发送", QString("广播 ◀ %1").arg(message), "#800080");
    }
    
    m_msgInput->clear();
}

void ServerWindow::onNewTcpConnection()
{
    QTcpSocket *client = m_tcpServer->nextPendingConnection();
    if (client) {
        m_tcpClients.append(client);
        connect(client, &QTcpSocket::readyRead, this, &ServerWindow::onTcpDataReceived);
        connect(client, &QTcpSocket::disconnected, this, &ServerWindow::onTcpClientDisconnected);
        
        QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
        appendMessage("连接", QString("%1 已连接").arg(clientInfo), "#008000");
        
        // 启用发送按钮
        m_sendBtn->setEnabled(true);
    }
}

void ServerWindow::onTcpDataReceived()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QByteArray data = client->readAll();
    QString message = QString::fromUtf8(data);
    QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
    appendMessage("接收", QString("%1 ▶ %2").arg(clientInfo).arg(message), "#0000FF");
}

void ServerWindow::onUdpDataReceived()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
        m_lastUdpAddress = sender.toString();
        m_lastUdpPort = senderPort;
        
        QString message = QString::fromUtf8(datagram);
        QString senderInfo = QString("%1:%2").arg(sender.toString()).arg(senderPort);
        appendMessage("接收", QString("%1 ▶ %2").arg(senderInfo).arg(message), "#0000FF");
        
        // 启用发送按钮
        m_sendBtn->setEnabled(true);
    }
}

void ServerWindow::onTcpClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QString clientInfo = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
    appendMessage("连接", QString("%1 已断开").arg(clientInfo), "#008000");
    
    m_tcpClients.removeOne(client);
    client->deleteLater();
    
    // 如果没有客户端了，禁用发送按钮
    if (m_tcpClients.isEmpty()) {
        m_sendBtn->setEnabled(false);
    }
}

void ServerWindow::startTcpServer()
{
    int port = m_portSpinBox->value();
    
    if (!m_tcpServer) {
        m_tcpServer = new QTcpServer(this);
    }
    
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        appendMessage("错误", QString("无法绑定端口 %1: %2").arg(port).arg(m_tcpServer->errorString()), "#FF0000");
        return;
    }
    
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ServerWindow::onNewTcpConnection);
    
    m_isRunning = true;
    m_startBtn->setText("停止服务");
    m_portSpinBox->setEnabled(false);
    m_protocolCombo->setEnabled(false);
    m_statusLabel->setText("服务状态: 运行中");
    appendMessage("系统", QString("TCP 服务已启动，监听端口: %1").arg(port), "#808080");
}

void ServerWindow::stopTcpServer()
{
    // 断开所有客户端
    for (QTcpSocket *client : m_tcpClients) {
        client->disconnectFromHost();
        if (client->state() != QTcpSocket::UnconnectedState) {
            client->waitForDisconnected(3000);
        }
    }
    
    qDeleteAll(m_tcpClients);
    m_tcpClients.clear();
    
    if (m_tcpServer) {
        m_tcpServer->close();
        disconnect(m_tcpServer, &QTcpServer::newConnection, this, &ServerWindow::onNewTcpConnection);
    }
    
    m_isRunning = false;
    m_startBtn->setText("启动服务");
    m_portSpinBox->setEnabled(true);
    m_protocolCombo->setEnabled(true);
    m_statusLabel->setText("服务状态: 已停止");
    m_sendBtn->setEnabled(false);
    appendMessage("系统", "服务已停止", "#808080");
}

void ServerWindow::startUdpServer()
{
    int port = m_portSpinBox->value();
    
    if (!m_udpSocket) {
        m_udpSocket = new QUdpSocket(this);
    }
    
    if (!m_udpSocket->bind(QHostAddress::Any, port)) {
        appendMessage("错误", QString("无法绑定端口 %1: %2").arg(port).arg(m_udpSocket->errorString()), "#FF0000");
        return;
    }
    
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &ServerWindow::onUdpDataReceived);
    
    m_isRunning = true;
    m_startBtn->setText("停止服务");
    m_portSpinBox->setEnabled(false);
    m_protocolCombo->setEnabled(false);
    m_statusLabel->setText("服务状态: 运行中");
    appendMessage("系统", QString("UDP 服务已启动，监听端口: %1").arg(port), "#808080");
}

void ServerWindow::stopUdpServer()
{
    if (m_udpSocket) {
        m_udpSocket->close();
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &ServerWindow::onUdpDataReceived);
    }
    
    m_isRunning = false;
    m_startBtn->setText("启动服务");
    m_portSpinBox->setEnabled(true);
    m_protocolCombo->setEnabled(true);
    m_statusLabel->setText("服务状态: 已停止");
    m_sendBtn->setEnabled(false);
    m_lastUdpPort = 0;
    m_lastUdpAddress.clear();
    appendMessage("系统", "服务已停止", "#808080");
}

void ServerWindow::appendMessage(const QString &type, const QString &content, const QString &color)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedMessage = QString("[%1] [%2] %3").arg(timestamp).arg(type).arg(content);
    
    if (!color.isEmpty()) {
        m_msgDisplay->append(QString("<span style=\"color:%1\">%2</span>").arg(color).arg(formattedMessage));
    } else {
        m_msgDisplay->append(formattedMessage);
    }
    
    // 滚动到底部
    QTextCursor cursor = m_msgDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_msgDisplay->setTextCursor(cursor);
}

void ServerWindow::clearAll()
{
    m_msgDisplay->clear();
    m_msgInput->clear();
    m_sendBtn->setEnabled(false);
}
