#include "clientwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Qt Network Debugger Client");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtNetworkDebugger");
    
    // 创建并显示主窗口
    ClientWindow clientWindow;
    clientWindow.show();
    
    return app.exec();
}
