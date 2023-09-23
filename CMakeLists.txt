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
        REQUIRED
)

enable_language("RC")

add_executable(ZJUConnectForWindows WIN32
        main.cpp
        mainwindow.cpp
        l2tpmode.cpp
        webloginmode.cpp
        zjuconnectmode.cpp
        mainwindow.h
        QrCodeGenerator/qrcodegenerator.cpp
        QrCodeGenerator/qrcodegenerator.h
        QrCodeGenerator/qrcodegen/qrcodegen.h
        QrCodeGenerator/qrcodegen/qrcodegen.cpp
        zjuconnectcontroller/zjuconnectcontroller.cpp
        zjuconnectcontroller/zjuconnectcontroller.h
        zjurulewindow/zjurulewindow.cpp
        zjurulewindow/zjurulewindow.h
        portforwardingwindow/portforwardingwindow.cpp
        portforwardingwindow/portforwardingwindow.h
        settingwindow/settingwindow.cpp
        settingwindow/settingwindow.h
        networkdetector/networkdetector.cpp
        networkdetector/networkdetector.h
        utils/utils.cpp
        utils/utils.h
        resource.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.rc
)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

add_subdirectory(singleapplication)

target_link_libraries(ZJUConnectForWindows
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Svg
        SingleApplication::SingleApplication
)

set(DEBUG_SUFFIX)
if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_SUFFIX "d")
endif ()
set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
    set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
    endif ()
endif ()
if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
endif ()
if (EXISTS "${QT_INSTALL_PATH}/plugins/tls/qcertonlybackend${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/tls/qcertonlybackend${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
endif ()
if (EXISTS "${QT_INSTALL_PATH}/plugins/tls/qopensslbackend${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/tls/qopensslbackend${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
endif ()
if (EXISTS "${QT_INSTALL_PATH}/plugins/tls/qschannelbackend${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/plugins/tls/qschannelbackend${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls/")
endif ()
foreach (QT_LIB Core Gui Widgets Network)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endforeach (QT_LIB)
