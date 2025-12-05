cmake_minimum_required(VERSION 3.16)

# Thiet lap du an Qt Widgets voi C++17 va kich hoat cac co che tu dong
# tao moc/ui/rc de don gian hoa khai bao file giao dien.
project(FansController VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    ui/mainwindow.cpp
    core/tuf_gaming_fx705ge.cpp
)

set(PROJECT_HEADERS
    inc/main.h
    ui/mainwindow.h
    core/tuf_gaming_fx705ge.h
)

# Dua stylesheet vao target de viec cai dat/phan phoi giu duoc file kieu dang.
set(PROJECT_RESOURCES
    style/mainwindow.css
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
)
