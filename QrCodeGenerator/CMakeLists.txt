cmake_minimum_required(VERSION 3.0)
project(QrCodeGenerator)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Svg
        REQUIRED
)

add_library(QrCodeGenerator STATIC
        QrCodeGenerator.cpp
        qrcodegen/qrcodegen.cpp
)

target_link_libraries(QrCodeGenerator
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
)
