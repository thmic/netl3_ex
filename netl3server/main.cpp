#include "serverwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Qt Network Debugger Server");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtNetworkDebugger");
    
    // 创建并显示主窗口
    ServerWindow serverWindow;
    serverWindow.show();
    
    return app.exec();
}
