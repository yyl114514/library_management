QT += core gui widgets sql

CONFIG += c++17 cmdline

# 生成可执行文件名称
TARGET = LibraryManagementSystem
TEMPLATE = app

# 按功能拆分后的源文件和头文件
SOURCES += main.cpp \
           mainwindow.cpp \
           dbmanager.cpp \
           bookmanager.cpp \
           readermanager.cpp \
           borrowmanager.cpp

HEADERS += mainwindow.h \
           dbmanager.h \
           bookmanager.h \
           readermanager.h \
           borrowmanager.h

# 确保SQLite支持
DEFINES += QT_DEPRECATED_WARNINGS

# 平台适配（删除/注释无效的图标引用）
win32 {
    # RC_ICONS = app.ico # 注释：无图标文件时务必注释这行，否则编译报错
}

unix {
    CONFIG += link_pkgconfig
}
