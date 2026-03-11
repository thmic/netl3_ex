cmake_minimum_required(VERSION 3.16)
project(QtNetworkDebuggerClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# 设置应用程序类型
if(WIN32)
    set(GUI_TYPE WIN32)
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} ${GUI_TYPE}
    main.cpp
    clientwindow.cpp
    clientwindow.h
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
