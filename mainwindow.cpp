#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 初始化数据库
    if (!DbManager::getInstance().initDatabase()) {
        QMessageBox::critical(this, "错误", "数据库初始化失败！");
        close();
        return;
    }

    // 创建功能模块实例
    m_bookMgr = new BookManager(this);
    m_readerMgr = new ReaderManager(this);
    m_borrowMgr = new BorrowManager(m_bookMgr, m_readerMgr, this);

    // 初始化UI
    initUI();

    // 绑定操作结果信号
    connect(m_bookMgr, &BookManager::operationSuccess, this, &MainWindow::handleOperationSuccess);
    connect(m_bookMgr, &BookManager::operationFailed, this, &MainWindow::handleOperationFailed);
    connect(m_readerMgr, &ReaderManager::operationSuccess, this, &MainWindow::handleOperationSuccess);
    connect(m_readerMgr, &ReaderManager::operationFailed, this, &MainWindow::handleOperationFailed);
    connect(m_borrowMgr, &BorrowManager::operationSuccess, this, &MainWindow::handleOperationSuccess);
    connect(m_borrowMgr, &BorrowManager::operationFailed, this, &MainWindow::handleOperationFailed);

    // 设置窗口属性
    setWindowTitle("图书与借阅管理系统");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    delete m_bookMgr;
    delete m_readerMgr;
    delete m_borrowMgr;
}

void MainWindow::initUI()
{
    // 主标签页
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 添加各功能标签页
    m_tabWidget->addTab(m_bookMgr->createBookWidget(), "图书管理");
    m_tabWidget->addTab(m_readerMgr->createReaderWidget(), "读者管理");
    m_tabWidget->addTab(m_borrowMgr->createBorrowWidget(), "借还书管理");
    m_tabWidget->addTab(m_borrowMgr->createStatWidget(), "借阅数据统计");
    m_tabWidget->addTab(m_borrowMgr->createOverdueWidget(), "逾期提醒");
}

void MainWindow::handleOperationSuccess(const QString& msg)
{
    QMessageBox::information(this, "成功", msg);
}

void MainWindow::handleOperationFailed(const QString& msg)
{
    QMessageBox::warning(this, "警告", msg);
}
