#include "clientwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QHostAddress>
#include <QTextCursor>
#include <QValidator>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tcpSocket(nullptr)
    , m_udpSocket(nullptr)
    , m_isConnected(false)
    , m_targetPort(3890)
{
    setupUI();
    setupConnections();
    clearAll();
}

ClientWindow::~ClientWindow()
{
    if (m_isConnected) {
        if (m_tcpSocket && m_tcpSocket->state() == QTcpSocket::ConnectedState) {
            disconnectFromServer();
        }
        if (m_udpSocket && m_udpSocket->isOpen()) {
            stopUdpClient();
        }
    }
    
    delete m_tcpSocket;
    delete m_udpSocket;
}

void ClientWindow::setupUI()
{
    // 设置主窗口
    setWindowTitle("Qt Network Debugger - 客户端");
    resize(600, 500);
    
    // 创建中心widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 控制区
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    m_protocolCombo = new QComboBox();
    m_protocolCombo->addItem("TCP Client");
    m_protocolCombo->addItem("UDP Client");
    
    m_addressInput = new QLineEdit();
    m_addressInput->setText("127.0.0.1");
    m_addressInput->setPlaceholderText("IP地址");
    
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(3890);
    
    m_connectBtn = new QPushButton("连接");
    
    controlLayout->addWidget(new QLabel("协议:"));
    controlLayout->addWidget(m_protocolCombo);
    controlLayout->addWidget(m_addressInput);
    controlLayout->addWidget(new QLabel(":"));
    controlLayout->addWidget(m_portSpinBox);
    controlLayout->addWidget(m_connectBtn);
    controlLayout->addStretch();
    
    // 状态栏
    m_statusLabel = new QLabel("连接状态: 未连接");
    
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

void ClientWindow::setupConnections()
{
    connect(m_protocolCombo, &QComboBox::currentIndexChanged,
            this, &ClientWindow::onProtocolChanged);
    connect(m_connectBtn, &QPushButton::clicked, this, &ClientWindow::onConnectClicked);
    connect(m_sendBtn, &QPushButton::clicked, this, &ClientWindow::onSendClicked);
    connect(m_msgInput, &QLineEdit::returnPressed, this, &ClientWindow::onSendClicked);
}

void ClientWindow::onProtocolChanged(int index)
{
    if (m_isConnected) {
        QMessageBox::warning(this, "警告", "连接状态下，无法切换协议！");
        // 恢复之前的选项
        m_protocolCombo->setCurrentIndex(index == 0 ? 1 : 0);
        return;
    }
    
    QString protocol = (index == 0) ? "TCP" : "UDP";
    appendMessage("系统", QString("协议已切换至 %1").arg(protocol));
    clearAll();
    
    // UDP模式下按钮文本改为"启动"
    if (index == 1) {
        m_connectBtn->setText("启动");
    } else {
        m_connectBtn->setText("连接");
    }
}

void ClientWindow::onConnectClicked()
{
    if (!m_isConnected) {
        int protocolIndex = m_protocolCombo->currentIndex();
        if (protocolIndex == 0) {
            // TCP Client
            connectToServer();
        } else {
            // UDP Client
            startUdpClient();
        }
    } else {
        int protocolIndex = m_protocolCombo->currentIndex();
        if (protocolIndex == 0) {
            // TCP Client
            disconnectFromServer();
        } else {
            // UDP Client
            stopUdpClient();
        }
    }
}

void ClientWindow::onSendClicked()
{
    QString message = m_msgInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    int protocolIndex = m_protocolCombo->currentIndex();
    if (protocolIndex == 0) {
        // TCP 发送
        if (m_tcpSocket && m_tcpSocket->state() == QTcpSocket::ConnectedState) {
            m_tcpSocket->write(message.toUtf8());
            appendMessage("发送", QString("▶ %1").arg(message), "#800080");
        } else {
            appendMessage("错误", "未连接到服务器");
            return;
        }
    } else {
        // UDP 发送
        if (!m_isConnected) {
            appendMessage("错误", "UDP客户端未启动");
            return;
        }
        
        m_udpSocket->writeDatagram(message.toUtf8(), QHostAddress(m_addressInput->text().trimmed()), m_portSpinBox->value());
        appendMessage("发送", QString("▶ %1").arg(message), "#800080");
        
        // 记录目标地址
        m_targetAddress = m_addressInput->text().trimmed();
        m_targetPort = m_portSpinBox->value();
    }
    
    m_msgInput->clear();
}

void ClientWindow::onTcpConnected()
{
    m_isConnected = true;
    m_connectBtn->setText("断开");
    m_addressInput->setEnabled(false);
    m_portSpinBox->setEnabled(false);
    m_protocolCombo->setEnabled(false);
    m_statusLabel->setText("连接状态: 已连接");
    m_sendBtn->setEnabled(true);
    appendMessage("系统", "连接成功", "#808080");
}

void ClientWindow::onTcpDisconnected()
{
    m_isConnected = false;
    m_connectBtn->setText("连接");
    m_addressInput->setEnabled(true);
    m_portSpinBox->setEnabled(true);
    m_protocolCombo->setEnabled(true);
    m_statusLabel->setText("连接状态: 未连接");
    m_sendBtn->setEnabled(false);
    appendMessage("系统", "连接已断开", "#808080");
}

void ClientWindow::onDataReceived()
{
    int protocolIndex = m_protocolCombo->currentIndex();
    if (protocolIndex == 0) {
        // TCP 数据接收
        if (m_tcpSocket) {
            QByteArray data = m_tcpSocket->readAll();
            QString message = QString::fromUtf8(data);
            appendMessage("接收", QString("◀ %1").arg(message), "#0000FF");
        }
    } else {
        // UDP 数据接收
        while (m_udpSocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(m_udpSocket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            
            m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            
            QString message = QString::fromUtf8(datagram);
            appendMessage("接收", QString("◀ %1").arg(message), "#0000FF");
        }
    }
}

void ClientWindow::connectToServer()
{
    QString address = m_addressInput->text().trimmed();
    quint16 port = m_portSpinBox->value();
    
    if (address.isEmpty()) {
        appendMessage("错误", "请输入服务器地址", "#FF0000");
        return;
    }
    
    if (!m_tcpSocket) {
        m_tcpSocket = new QTcpSocket(this);
        connect(m_tcpSocket, &QTcpSocket::connected, this, &ClientWindow::onTcpConnected);
        connect(m_tcpSocket, &QTcpSocket::disconnected, this, &ClientWindow::onTcpDisconnected);
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &ClientWindow::onDataReceived);
        connect(m_tcpSocket, &QTcpSocket::errorOccurred,
                [=](QAbstractSocket::SocketError error) {
                    Q_UNUSED(error);
                    appendMessage("错误", QString("连接失败: %1").arg(m_tcpSocket->errorString()), "#FF0000");
                    m_isConnected = false;
                });
    }
    
    appendMessage("系统", QString("正在连接 %1:%2...").arg(address).arg(port), "#808080");
    m_tcpSocket->connectToHost(QHostAddress(address), port);
}

void ClientWindow::disconnectFromServer()
{
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->state() != QTcpSocket::UnconnectedState) {
            m_tcpSocket->waitForDisconnected(3000);
        }
    }
}

void ClientWindow::startUdpClient()
{
    quint16 targetPort = m_portSpinBox->value();
    quint16 localPort = targetPort + 1; // 使用目标端口+1作为本地端口
    
    if (!m_udpSocket) {
        m_udpSocket = new QUdpSocket(this);
    }
    
    // 绑定本地端口用于接收（目标端口+1）
    if (!m_udpSocket->bind(QHostAddress::Any, localPort)) {
        appendMessage("错误", QString("无法绑定本地端口 %1: %2").arg(localPort).arg(m_udpSocket->errorString()), "#FF0000");
        return;
    }
    
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &ClientWindow::onDataReceived);
    
    m_isConnected = true;
    m_connectBtn->setText("停止");
    m_addressInput->setEnabled(true); // UDP模式下地址可编辑
    m_portSpinBox->setEnabled(false);
    m_protocolCombo->setEnabled(false);
    m_statusLabel->setText("连接状态: 运行中");
    m_sendBtn->setEnabled(true);
    appendMessage("系统", QString("UDP客户端已启动，目标端口: %1，本地端口: %2").arg(targetPort).arg(localPort), "#808080");
}

void ClientWindow::stopUdpClient()
{
    if (m_udpSocket) {
        m_udpSocket->close();
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &ClientWindow::onDataReceived);
    }
    
    m_isConnected = false;
    m_connectBtn->setText("启动");
    m_addressInput->setEnabled(true);
    m_portSpinBox->setEnabled(true);
    m_protocolCombo->setEnabled(true);
    m_statusLabel->setText("连接状态: 未连接");
    m_sendBtn->setEnabled(false);
    appendMessage("系统", "UDP客户端已停止", "#808080");
}

void ClientWindow::appendMessage(const QString &type, const QString &content, const QString &color)
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

void ClientWindow::clearAll()
{
    m_msgDisplay->clear();
    m_msgInput->clear();
    m_sendBtn->setEnabled(false);
}
