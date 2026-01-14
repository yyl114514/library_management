#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMessageBox>
#include "dbmanager.h"
#include "bookmanager.h"
#include "readermanager.h"
#include "borrowmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 处理各模块的操作结果
    void handleOperationSuccess(const QString& msg);
    void handleOperationFailed(const QString& msg);

private:
    // 初始化UI
    void initUI();

    // 模块实例
    BookManager *m_bookMgr;
    ReaderManager *m_readerMgr;
    BorrowManager *m_borrowMgr;

    QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H
