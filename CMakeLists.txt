cmake_minimum_required(VERSION 3.12)
project(HITszConnectForWindows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(NIGHTLY_BUILD    "Set if built nightly.")

if(NIGHTLY_BUILD)
    set(PROJ_VER "${GIT_HASH}-nightly")
elseif(DEFINED REL_VER)
    set(PROJ_VER "${REL_VER}")
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

set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    zjuconnectmode.cpp
    zjuconnectcontroller/zjuconnectcontroller.cpp
    portforwardingwindow/portforwardingwindow.cpp
    settingwindow/settingwindow.cpp
    loginwindow/loginwindow.cpp
    utils/utils.cpp
    utils/proxysettings.cpp
    resource.qrc
)

if (WIN32)
    message("Building for Windows")

    enable_language("RC")

    add_executable(${PROJECT_NAME} WIN32
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.rc
    )

    set(PLATFORM_LIBS wininet rasapi32)
elseif(APPLE)
    message("Building for MacOS")
    
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    
    set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.icns)
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${app_icon_macos}
    )
else()
    add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
    )
endif()

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
        ${PLATFORM_LIBS}
)
