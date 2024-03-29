cmake_minimum_required(VERSION 3.0)
project(ZJUConnectForWindows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        Svg
        Core5Compat
        REQUIRED
)

enable_language("RC")

add_executable(ZJUConnectForWindows WIN32
        main.cpp
        mainwindow.cpp
        l2tpmode.cpp
        webloginmode.cpp
        zjuconnectmode.cpp
        zjuconnectcontroller/zjuconnectcontroller.cpp
        portforwardingwindow/portforwardingwindow.cpp
        settingwindow/settingwindow.cpp
        networkdetector/networkdetector.cpp
        utils/utils.cpp
        resource.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.rc
)

add_subdirectory(QrCodeGenerator)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

add_subdirectory(singleapplication)

target_link_libraries(ZJUConnectForWindows
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Svg
        Qt::Core5Compat
        SingleApplication::SingleApplication
        QrCodeGenerator
        wininet
        rasapi32
)
