cmake_minimum_required(VERSION 3.12)
project(HITszConnectForWindows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(NIGHTLY_BUILD    "Set if built nightly.")

if(NIGHTLY_BUILD)
    set(PROJ_VER "${GIT_HASH}-nightly")
elseif(DEFINED GIT_TAG)
    set(PROJ_VER "${GIT_TAG}")
else()
    set(PROJ_VER "dev")
endif()

add_compile_definitions(PROJ_VER="${PROJ_VER}")
message("PROJ_VER=${PROJ_VER}")

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

add_executable(${PROJECT_NAME} WIN32
        main.cpp
        mainwindow.cpp
        zjuconnectmode.cpp
        zjuconnectcontroller/zjuconnectcontroller.cpp
        portforwardingwindow/portforwardingwindow.cpp
        settingwindow/settingwindow.cpp
        loginwindow/loginwindow.cpp
        utils/utils.cpp
        resource.qrc
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.rc
)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

add_subdirectory(singleapplication)

target_link_libraries(${PROJECT_NAME}
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Svg
        Qt::Core5Compat
        SingleApplication::SingleApplication
        wininet
        rasapi32
)
