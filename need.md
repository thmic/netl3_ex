# Qt TCP/UDP 网络调试工具需求文档

## 1. 项目概述

### 1.1 项目名称
Qt Network Debugger（Qt 网络调试助手）

### 1.2 项目目标
开发一款基于 Qt 框架的 L3 层网络调试工具，支持 TCP 和 UDP 协议，包含服务端和客户端两个独立模块，用于网络通信测试与调试。

### 1.3 技术栈
- **开发框架**: Qt 6.x / Qt 5.15+
- **编程语言**: C++17
- **网络模块**: QtNetwork (QTcpServer, QTcpSocket, QUdpSocket)
- **UI 框架**: Qt Widgets
- **构建工具**: CMake / qmake

---

## 2. 功能架构
┌─────────────────────────────────────────────────────────────┐
│                    Qt Network Debugger                       │
├─────────────────────────────┬───────────────────────────────┤
│         服务端 (Server)      │         客户端 (Client)        │
├─────────────────────────────┼───────────────────────────────┤
│  • 协议选择 (TCP/UDP)        │  • 协议选择 (TCP/UDP)          │
│  • 端口监听 (默认3890)       │  • 目标IP (默认127.0.0.1)      │
│  • 多客户端管理 (TCP)        │  • 目标端口 (默认3890)         │
│  • 消息收发与展示            │  • 连接管理                    │
│  • 服务状态控制              │  • 消息收发与展示              │
└─────────────────────────────┴───────────────────────────────┘

---

## 3. 服务端 (Server) 详细需求

### 3.1 界面布局
┌────────────────────────────────────────────┐
│  [协议选择▼] [端口: 3890    ] [启动服务]    │  ← 控制区
├────────────────────────────────────────────┤
│  服务状态: 已停止                           │  ← 状态栏
├────────────────────────────────────────────┤
│  [2026-03-11 10:45:32] [系统] 服务已启动    │  ← 消息展示窗口
│  [2026-03-11 10:45:35] [接收] 127.0.0.1:54321 ▶ Hello Server
│  [2026-03-11 10:45:40] [发送] 127.0.0.1:54321 ◀ Welcome Client
│  ...                                       │
│                                            │
├────────────────────────────────────────────┤
│  [输入消息...                    ] [发送]   │  ← 输入区
└────────────────────────────────────────────┘

### 3.2 控件清单

| 控件类型 | 控件名称 | 属性/默认值 | 功能说明 |
|---------|---------|------------|---------|
| QComboBox | protocolCombo | TCP/UDP 选项 | 协议选择，切换时重置服务状态 |
| QSpinBox | portSpinBox | 范围 1-65535, 默认 3890 | 服务监听端口 |
| QPushButton | startBtn | 文本: "启动服务" | 启动/停止服务切换按钮 |
| QTextBrowser | msgDisplay | 只读, 支持时间戳 | 显示系统日志、收发消息 |
| QLineEdit | msgInput | 占位符: "输入消息..." | 消息输入框 |
| QPushButton | sendBtn | 文本: "发送" | 发送消息到客户端 |
| QLabel | statusLabel | 文本: "服务状态: 已停止" | 显示当前服务状态 |

### 3.3 功能需求

#### 3.3.1 协议选择 (QComboBox)
- **选项**: TCP Server, UDP Server
- **行为**: 
  - 服务运行中禁止切换协议
  - 切换协议时清空消息窗口并提示"协议已切换至 XXX"

#### 3.3.2 端口设置 (QSpinBox)
- **范围**: 1 - 65535
- **默认值**: 3890
- **行为**:
  - 服务运行中禁止修改端口
  - 端口被占用时提示错误信息

#### 3.3.3 服务控制 (QPushButton - startBtn)
- **状态机**:
已停止 → [点击] → 启动中 → 运行中 (按钮文本变为"停止服务")
运行中 → [点击] → 停止中 → 已停止 (按钮文本变为"启动服务")
- **启动流程**:
1. 验证端口有效性
2. 根据协议创建 QTcpServer 或 QUdpSocket
3. 绑定端口并开始监听
4. 日志输出: `[系统] TCP/UDP 服务已启动，监听端口: XXXX`
- **停止流程**:
1. 断开所有客户端连接 (TCP)
2. 关闭 Socket
3. 日志输出: `[系统] 服务已停止`

#### 3.3.4 消息展示窗口 (QTextBrowser)
- **消息格式**: `[YYYY-MM-DD HH:MM:SS] [类型] 内容`
- **消息类型及颜色**:
- `[系统]`: 灰色 (#808080) - 服务启动/停止/错误信息
- `[连接]`: 绿色 (#008000) - 客户端连接/断开 (TCP)
- `[接收]`: 蓝色 (#0000FF) - 收到客户端消息
- `[发送]`: 紫色 (#800080) - 发送给客户端的消息
- `[错误]`: 红色 (#FF0000) - 网络错误

- **TCP 消息格式**:


[接收] 192.168.1.100:54321 ▶ 消息内容
[发送] 192.168.1.100:54321 ◀ 消息内容

- **UDP 消息格式**:

[接收] 192.168.1.100:54321 ▶ 消息内容
[发送] 广播 ◀ 消息内容  (UDP发送为广播/最后接收地址)


#### 3.3.5 消息发送功能
- **TCP 模式**:
- 支持多客户端连接
- 发送方式选项: 发送到选中客户端 / 广播所有客户端
- 无客户端连接时禁用发送按钮并提示"无客户端连接"
- **UDP 模式**:
- 发送到最后一次接收消息的地址
- 或支持输入目标 IP:Port 发送
- **快捷操作**: 支持 Enter 键发送消息

#### 3.3.6 TCP 特定功能
- **客户端列表**: 右侧或下拉框显示已连接客户端 (IP:Port)
- **连接管理**: 支持查看、选中、断开指定客户端

---

## 4. 客户端 (Client) 详细需求

### 4.1 界面布局
┌────────────────────────────────────────────┐
│  [协议选择▼] [127.0.0.1:3890    ] [连接]    │  ← 控制区
├────────────────────────────────────────────┤
│  连接状态: 未连接                           │  ← 状态栏
├────────────────────────────────────────────┤
│  [2026-03-11 10:45:32] [系统] 正在连接...   │  ← 消息展示窗口
│  [2026-03-11 10:45:33] [系统] 连接成功      │
│  [2026-03-11 10:45:35] [接收] ◀ Welcome Client
│  [2026-03-11 10:45:40] [发送] ▶ Hello Server
│  ...                                       │
│                                            │
├────────────────────────────────────────────┤
│  [输入消息...                    ] [发送]   │  ← 输入区
└────────────────────────────────────────────┘

### 4.2 控件清单

| 控件类型 | 控件名称 | 属性/默认值 | 功能说明 |
|---------|---------|------------|---------|
| QComboBox | protocolCombo | TCP Client/UDP Client | 协议选择 |
| QLineEdit | addressInput | 默认: "127.0.0.1" | 目标服务器 IP |
| QSpinBox | portSpinBox | 默认: 3890 | 目标服务器端口 |
| QPushButton | connectBtn | 文本: "连接" | 连接/断开切换按钮 |
| QTextBrowser | msgDisplay | 只读 | 显示连接状态、收发消息 |
| QLineEdit | msgInput | 占位符: "输入消息..." | 消息输入框 |
| QPushButton | sendBtn | 文本: "发送" | 发送消息到服务端 |
| QLabel | statusLabel | 文本: "连接状态: 未连接" | 显示连接状态 |

### 4.3 功能需求

#### 4.3.1 协议选择 (QComboBox)
- **选项**: TCP Client, UDP Client
- **行为**:
  - 连接状态下禁止切换协议
  - UDP 模式下"连接"按钮变为"启动"（仅绑定本地端口）

#### 4.3.2 地址与端口
- **IP 输入**: 支持 IPv4 格式验证，默认 127.0.0.1
- **端口**: 默认 3890，范围 1-65535
- **连接状态下**: 禁止修改地址和端口

#### 4.3.3 连接控制 (QPushButton - connectBtn)
- **TCP 模式状态机**:
未连接 → [点击] → 连接中 → 已连接 (按钮文本变为"断开")
已连接 → [点击] → 断开中 → 未连接 (按钮文本变为"连接")

- **UDP 模式**:
- 点击"启动"绑定本地端口（使用相同端口接收）
- 按钮变为"停止"停止接收

- **连接日志**:
- `[系统] 正在连接 127.0.0.1:3890...`
- `[系统] 连接成功` / `[系统] 连接失败: 拒绝连接`
- `[系统] 连接已断开`

#### 4.3.4 UDP 特定功能
- **端口复用**: 使用与发送相同的端口接收服务端回复
- **目标地址记忆**: 发送时记录目标地址，接收时显示来源地址

#### 4.3.5 消息展示 (QTextBrowser)
- **消息格式**:
[接收] ◀ 消息内容        (来自服务端)
[发送] ▶ 消息内容        (发送到服务端)
[系统] 消息内容          (系统提示)
[错误] 消息内容          (错误信息)


#### 4.3.6 消息发送
- **TCP**: 通过已建立的连接发送
- **UDP**: 发送到指定的 IP:Port
- **未连接状态**: 禁用发送按钮
- **快捷操作**: 支持 Enter 键发送

---

## 5. 消息格式规范

### 5.1 时间戳格式
[YYYY-MM-DD HH:MM:SS]  例如: [2026-03-11 10:45:32]

### 5.2 消息类型标识
| 类型 | 标识符 | 颜色 | 使用场景 |
|-----|-------|------|---------|
| 系统 | `[系统]` | 灰色 | 服务启动/停止、连接状态变化 |
| 连接 | `[连接]` | 绿色 | TCP 客户端连接/断开 |
| 接收 | `[接收]` | 蓝色 | 收到对方发送的数据 |
| 发送 | `[发送]` | 紫色 | 本端发送的数据 |
| 错误 | `[错误]` | 红色 | 网络错误、操作失败 |

### 5.3 方向指示符
- `▶` : 接收到的消息
- `◀` : 发送的消息

---

## 6. 技术实现要点

### 6.1 类结构设计

```cpp
// 服务端
class ServerWindow : public QMainWindow {
    Q_OBJECT
private:
    QTcpServer *tcpServer;
    QList<QTcpSocket*> tcpClients;
    QUdpSocket *udpSocket;
    
    void setupUI();
    void setupConnections();
    
private slots:
    void onProtocolChanged(int index);
    void onStartClicked();
    void onSendClicked();
    void onNewTcpConnection();
    void onTcpDataReceived();
    void onUdpDataReceived();
};

// 客户端
class ClientWindow : public QMainWindow {
    Q_OBJECT
private:
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;
    
    void setupUI();
    void setupConnections();
    
private slots:
    void onProtocolChanged(int index);
    void onConnectClicked();
    void onSendClicked();
    void onTcpConnected();
    void onTcpDisconnected();
    void onDataReceived();
};
### 6.2 信号与槽设计
Table
发送者	信号	接收者	槽	说明
protocolCombo	currentIndexChanged	this	onProtocolChanged	协议切换
startBtn/connectBtn	clicked	this	onStartClicked/onConnectClicked	启动/连接
sendBtn	clicked	this	onSendClicked	发送消息
msgInput	returnPressed	this	onSendClicked	回车发送
tcpServer	newConnection	this	onNewTcpConnection	新 TCP 连接
tcpSocket	readyRead	this	onDataReceived	TCP 数据到达
udpSocket	readyRead	this	onUdpDataReceived	UDP 数据到达
### 6.3 关键代码逻辑