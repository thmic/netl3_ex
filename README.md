# Qt Network Debugger

一个基于 Qt 框架开发的 L3 层网络调试工具，支持 TCP 和 UDP 协议，包含服务端和客户端两个独立模块，用于网络通信测试与调试。

## 功能特性

### 服务端 (Server)
- 支持 TCP 和 UDP 两种协议
- 可配置监听端口（默认 3890）
- 实时显示连接状态和消息收发
- 支持多客户端连接管理（TCP 模式）
- 消息格式化显示，包含时间戳和类型标识
- 支持广播发送消息到所有客户端

### 客户端 (Client)
- 支持 TCP 和 UDP 两种协议
- 可配置目标服务器地址和端口
- 实时显示连接状态和消息收发
- 支持 Enter 键快速发送消息
- UDP 模式支持端口复用接收服务端回复

## 技术栈

- **开发框架**: Qt 6.x / Qt 5.15+
- **编程语言**: C++17
- **网络模块**: QtNetwork (QTcpServer, QTcpSocket, QUdpSocket)
- **UI 框架**: Qt Widgets
- **构建工具**: CMake / qmake

## 项目结构

```
netl3_ex/
├── netl3server/          # 服务端模块
│   ├── serverwindow.h    # 服务端窗口类头文件
│   ├── serverwindow.cpp  # 服务端窗口类实现
│   ├── main.cpp          # 服务端入口
│   ├── CMakeLists.txt    # CMake 构建文件
│   └── netl3server.pro   # qmake 构建文件
├── netl3client/          # 客户端模块
│   ├── clientwindow.h    # 客户端窗口类头文件
│   ├── clientwindow.cpp  # 客户端窗口类实现
│   ├── main.cpp          # 客户端入口
│   ├── CMakeLists.txt    # CMake 构建文件
│   └── netl3client.pro   # qmake 构建文件
└── README.md             # 项目说明文档
```

## 编译构建

### 使用 CMake

```bash
# 构建服务端
cd netl3server/build
cmake ..
make

# 构建客户端
cd ../netl3client/build
cmake ..
make
```

### 使用 qmake

```bash
# 构建服务端
cd netl3server
qmake netl3server.pro
make

# 构建客户端
cd ../netl3client
qmake netl3client.pro
make
```

## 使用说明

### 服务端使用

1. 启动服务端程序
2. 选择协议类型（TCP Server / UDP Server）
3. 设置监听端口（默认 3890）
4. 点击"启动服务"按钮
5. 服务启动后，状态显示为"运行中"
6. 在消息输入框中输入内容，点击"发送"可向客户端发送消息

### 客户端使用

1. 启动客户端程序
2. 选择协议类型（TCP Client / UDP Client）
3. 设置目标服务器地址（默认 127.0.0.1）和端口（默认 3890）
4. 点击"连接"（TCP）或"启动"（UDP）按钮
5. 连接成功后，状态显示为"已连接"或"运行中"
6. 在消息输入框中输入内容，点击"发送"可向服务端发送消息

### 消息格式

所有消息都包含时间戳和类型标识：

```
[2026-03-11 10:45:32] [系统] 服务已启动
[2026-03-11 10:45:35] [接收] 127.0.0.1:54321 ▶ Hello Server
[2026-03-11 10:45:40] [发送] 127.0.0.1:54321 ◀ Welcome Client
```

### 消息类型颜色标识

- **系统** (灰色): 服务启动/停止、连接状态变化
- **连接** (绿色): TCP 客户端连接/断开
- **接收** (蓝色): 收到对方发送的数据
- **发送** (紫色): 本端发送的数据
- **错误** (红色): 网络错误、操作失败

## 协议说明

### TCP 模式

**服务端**:
- 支持多客户端同时连接
- 可向单个客户端或所有客户端广播消息
- 自动管理客户端连接和断开

**客户端**:
- 建立与服务端的持久连接
- 支持实时双向通信

### UDP 模式

**服务端**:
- 监听指定端口接收 UDP 数据报
- 可向最后接收消息的地址发送回复

**客户端**:
- 使用目标端口+1 作为本地端口接收服务端回复
- 支持无连接的数据传输

## 开发说明

### 类结构设计

#### 服务端类 (ServerWindow)
- `QTcpServer *tcpServer`: TCP 服务器实例
- `QList<QTcpSocket*> tcpClients`: TCP 客户端列表
- `QUdpSocket *udpSocket`: UDP 套接字实例
- 关键槽函数：
  - `onNewTcpConnection()`: 处理新 TCP 连接
  - `onTcpDataReceived()`: 处理 TCP 数据接收
  - `onUdpDataReceived()`: 处理 UDP 数据接收

#### 客户端类 (ClientWindow)
- `QTcpSocket *tcpSocket`: TCP 套接字实例
- `QUdpSocket *udpSocket`: UDP 套接字实例
- 关键槽函数：
  - `onTcpConnected()`: 处理 TCP 连接成功
  - `onTcpDisconnected()`: 处理 TCP 连接断开
  - `onDataReceived()`: 处理数据接收

### 信号与槽机制

项目使用 Qt 的信号与槽机制处理网络事件：

- `QTcpServer::newConnection` → `onNewTcpConnection()`
- `QTcpSocket::readyRead` → `onTcpDataReceived()`
- `QUdpSocket::readyRead` → `onUdpDataReceived()`
- `QLineEdit::returnPressed` → `onSendClicked()`

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。

## 联系方式

如有问题或建议，请通过 GitHub Issues 联系。