#include "borrowmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

BorrowManager::BorrowManager(BookManager* bookMgr, ReaderManager* readerMgr, QObject *parent)
    : QObject(parent), m_bookMgr(bookMgr), m_readerMgr(readerMgr)
{
    initBorrowModel();
}

BorrowManager::~BorrowManager()
{
    delete m_borrowModel;
    delete m_statModel;
    delete m_overdueModel;
}

void BorrowManager::initBorrowModel()
{
    m_borrowModel = new QSqlTableModel(this, DbManager::getInstance().getDatabase());
    m_borrowModel->setTable("borrows");
    m_borrowModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_borrowModel->select();

    // 设置列名
    m_borrowModel->setHeaderData(0, Qt::Horizontal, "借阅ID");
    m_borrowModel->setHeaderData(1, Qt::Horizontal, "图书ID");
    m_borrowModel->setHeaderData(2, Qt::Horizontal, "读者ID");
    m_borrowModel->setHeaderData(3, Qt::Horizontal, "借阅时间");
    m_borrowModel->setHeaderData(4, Qt::Horizontal, "归还时间");
    m_borrowModel->setHeaderData(5, Qt::Horizontal, "状态(0=未还/1=已还)");

    m_statModel = new QSqlQueryModel(this);
    m_overdueModel = new QSqlQueryModel(this);
}

void BorrowManager::loadComboData()
{
    // 加载图书ID到下拉框
    m_bookIdCombo->clear();
    QSqlQuery bookIdQuery("SELECT id, book_name FROM books");
    while (bookIdQuery.next()) {
        QString item = QString("%1 - %2").arg(bookIdQuery.value(0).toInt()).arg(bookIdQuery.value(1).toString());
        m_bookIdCombo->addItem(item);
    }

    // 加载读者ID到下拉框
    m_readerIdCombo->clear();
    QSqlQuery readerIdQuery("SELECT id, reader_name FROM readers");
    while (readerIdQuery.next()) {
        QString item = QString("%1 - %2").arg(readerIdQuery.value(0).toInt()).arg(readerIdQuery.value(1).toString());
        m_readerIdCombo->addItem(item);
    }
}

QWidget* BorrowManager::createBorrowWidget()
{
    QWidget *borrowWidget = new QWidget;
    QVBoxLayout *borrowVLayout = new QVBoxLayout(borrowWidget);

    // 借还书表单
    QGroupBox *borrowFormGroup = new QGroupBox("借还书操作");
    QFormLayout *borrowFormLayout = new QFormLayout;

    m_bookIdCombo = new QComboBox;
    m_readerIdCombo = new QComboBox;
    loadComboData(); // 加载下拉框数据

    m_borrowStatusLabel = new QLabel("状态：未操作");
    borrowFormLayout->addRow("选择图书：", m_bookIdCombo);
    borrowFormLayout->addRow("选择读者：", m_readerIdCombo);
    borrowFormLayout->addRow(m_borrowStatusLabel);

    // 借还书按钮
    QHBoxLayout *borrowBtnLayout = new QHBoxLayout;
    QPushButton *borrowBookBtn = new QPushButton("借书");
    QPushButton *returnBookBtn = new QPushButton("还书");
    borrowBtnLayout->addWidget(borrowBookBtn);
    borrowBtnLayout->addWidget(returnBookBtn);
    borrowBtnLayout->addStretch();

    // 借阅记录表格
    m_borrowTableView = new QTableView;
    m_borrowTableView->setModel(m_borrowModel);
    m_borrowTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_borrowTableView->resizeColumnsToContents();

    // 组装界面
    borrowFormGroup->setLayout(borrowFormLayout);
    borrowVLayout->addWidget(borrowFormGroup);
    borrowVLayout->addLayout(borrowBtnLayout);
    borrowVLayout->addWidget(m_borrowTableView);

    // 绑定槽函数
    connect(borrowBookBtn, &QPushButton::clicked, this, &BorrowManager::onBorrowBook);
    connect(returnBookBtn, &QPushButton::clicked, this, &BorrowManager::onReturnBook);

    return borrowWidget;
}

QWidget* BorrowManager::createStatWidget()
{
    QWidget *statWidget = new QWidget;
    QVBoxLayout *statVLayout = new QVBoxLayout(statWidget);

    QPushButton *statBtn = new QPushButton("刷新统计数据");
    m_statTextEdit = new QTextEdit;
    m_statTextEdit->setReadOnly(true);

    statVLayout->addWidget(statBtn);
    statVLayout->addWidget(m_statTextEdit);

    connect(statBtn, &QPushButton::clicked, this, &BorrowManager::onStatisticData);

    return statWidget;
}

QWidget* BorrowManager::createOverdueWidget()
{
    QWidget *overdueWidget = new QWidget;
    QVBoxLayout *overdueVLayout = new QVBoxLayout(overdueWidget);

    QPushButton *overdueBtn = new QPushButton("查询逾期记录");
    m_overdueTableView = new QTableView;
    m_overdueTableView->setModel(m_overdueModel);
    m_overdueTableView->resizeColumnsToContents();

    overdueVLayout->addWidget(overdueBtn);
    overdueVLayout->addWidget(m_overdueTableView);

    connect(overdueBtn, &QPushButton::clicked, this, &BorrowManager::onCheckOverdue);

    return overdueWidget;
}

void BorrowManager::refreshBorrowModel()
{
    m_borrowModel->submitAll();
    m_borrowModel->select();
    loadComboData(); // 重新加载下拉框数据
}

void BorrowManager::onBorrowBook()
{
    // 解析选中的图书ID和读者ID
    QString bookText = m_bookIdCombo->currentText();
    QString readerText = m_readerIdCombo->currentText();
    if (bookText.isEmpty() || readerText.isEmpty()) {
        emit operationFailed("请选择有效的图书和读者！");
        return;
    }

    int bookId = bookText.split(" - ").first().toInt();
    int readerId = readerText.split(" - ").first().toInt();

    // 检查图书库存
    QSqlQuery stockQuery;
    stockQuery.prepare("SELECT stock FROM books WHERE id=?");
    stockQuery.addBindValue(bookId);
    if (stockQuery.exec() && stockQuery.next()) {
        int stock = stockQuery.value(0).toInt();
        if (stock <= 0) {
            emit operationFailed("该图书库存不足，无法借阅！");
            return;
        }
    } else {
        emit operationFailed("查询图书库存失败：" + stockQuery.lastError().text());
        return;
    }

    // 插入借阅记录
    QSqlQuery borrowQuery;
    borrowQuery.prepare("INSERT INTO borrows (book_id, reader_id, borrow_time, status) VALUES (?, ?, datetime('now'), 0)");
    borrowQuery.addBindValue(bookId);
    borrowQuery.addBindValue(readerId);

    // 减少图书库存
    QSqlQuery updateStockQuery;
    updateStockQuery.prepare("UPDATE books SET stock = stock - 1 WHERE id=?");
    updateStockQuery.addBindValue(bookId);

    // 事务处理
    DbManager& dbMgr = DbManager::getInstance();
    if (dbMgr.beginTransaction()) {
        if (borrowQuery.exec() && updateStockQuery.exec()) {
            dbMgr.commitTransaction();
            m_borrowStatusLabel->setText("状态：借书成功");
            refreshBorrowModel();
            m_bookMgr->refreshModel(); // 刷新图书库存
            emit operationSuccess("借书成功！");
        } else {
            dbMgr.rollbackTransaction();
            emit operationFailed("借书失败：" + borrowQuery.lastError().text());
            m_borrowStatusLabel->setText("状态：借书失败");
        }
    } else {
        emit operationFailed("事务启动失败！");
    }
}

void BorrowManager::onReturnBook()
{
    // 获取选中的借阅记录
    QModelIndexList selected = m_borrowTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        emit operationFailed("请先选中要归还的借阅记录！");
        return;
    }

    int row = selected.first().row();
    int borrowId = m_borrowModel->data(m_borrowModel->index(row, 0)).toInt();
    int bookId = m_borrowModel->data(m_borrowModel->index(row, 1)).toInt();
    int status = m_borrowModel->data(m_borrowModel->index(row, 5)).toInt();

    if (status == 1) {
        emit operationFailed("该记录已归还，无需重复操作！");
        return;
    }

    // 更新借阅记录
    QSqlQuery returnQuery;
    returnQuery.prepare("UPDATE borrows SET return_time=datetime('now'), status=1 WHERE id=?");
    returnQuery.addBindValue(borrowId);

    // 增加图书库存
    QSqlQuery updateStockQuery;
    updateStockQuery.prepare("UPDATE books SET stock = stock + 1 WHERE id=?");
    updateStockQuery.addBindValue(bookId);

    // 事务处理
    DbManager& dbMgr = DbManager::getInstance();
    if (dbMgr.beginTransaction()) {
        if (returnQuery.exec() && updateStockQuery.exec()) {
            dbMgr.commitTransaction();
            m_borrowStatusLabel->setText("状态：还书成功");
            refreshBorrowModel();
            m_bookMgr->refreshModel();
            emit operationSuccess("还书成功！");
        } else {
            dbMgr.rollbackTransaction();
            emit operationFailed("还书失败：" + returnQuery.lastError().text());
            m_borrowStatusLabel->setText("状态：还书失败");
        }
    } else {
        emit operationFailed("事务启动失败！");
    }
}

void BorrowManager::onStatisticData()
{
    QString statText;

    // 1. 总借阅次数
    QSqlQuery totalBorrowQuery("SELECT COUNT(*) FROM borrows");
    if (totalBorrowQuery.exec() && totalBorrowQuery.next()) {
        statText += QString("1. 总借阅次数：%1\n").arg(totalBorrowQuery.value(0).toInt());
    }

    // 2. 未归还次数
    QSqlQuery unReturnedQuery("SELECT COUNT(*) FROM borrows WHERE status=0");
    if (unReturnedQuery.exec() && unReturnedQuery.next()) {
        statText += QString("2. 未归还次数：%1\n").arg(unReturnedQuery.value(0).toInt());
    }

    // 3. 已归还次数
    QSqlQuery returnedQuery("SELECT COUNT(*) FROM borrows WHERE status=1");
    if (returnedQuery.exec() && returnedQuery.next()) {
        statText += QString("3. 已归还次数：%1\n").arg(returnedQuery.value(0).toInt());
    }

    // 4. 各图书借阅次数
    statText += "\n4. 各图书借阅次数：\n";
    QSqlQuery bookBorrowQuery(R"(
        SELECT b.book_name, COUNT(br.id) AS borrow_count
        FROM books b
        LEFT JOIN borrows br ON b.id = br.book_id
        GROUP BY b.id, b.book_name
        ORDER BY borrow_count DESC
    )");
    while (bookBorrowQuery.next()) {
        statText += QString("   %1：%2次\n").arg(bookBorrowQuery.value(0).toString()).arg(bookBorrowQuery.value(1).toInt());
    }

    m_statTextEdit->setText(statText);
    emit operationSuccess("统计数据刷新成功！");
}

void BorrowManager::onCheckOverdue()
{
    // 查询逾期未还的记录
    QString overdueSql = R"(
        SELECT br.id, b.book_name, r.reader_name, br.borrow_time,
        julianday('now') - julianday(br.borrow_time) AS overdue_days
        FROM borrows br
        LEFT JOIN books b ON br.book_id = b.id
        LEFT JOIN readers r ON br.reader_id = r.id
        WHERE br.status=0 AND julianday('now') - julianday(br.borrow_time) > ?
    )";

    QSqlQuery query;
    query.prepare(overdueSql);
    query.addBindValue(BORROW_LIMIT_DAYS);
    query.exec();

    m_overdueModel->setQuery(query);
    // 设置列名
    m_overdueModel->setHeaderData(0, Qt::Horizontal, "借阅ID");
    m_overdueModel->setHeaderData(1, Qt::Horizontal, "图书名称");
    m_overdueModel->setHeaderData(2, Qt::Horizontal, "读者姓名");
    m_overdueModel->setHeaderData(3, Qt::Horizontal, "借阅时间");
    m_overdueModel->setHeaderData(4, Qt::Horizontal, "逾期天数");

    m_overdueTableView->setModel(m_overdueModel);
    m_overdueTableView->resizeColumnsToContents();

    // 提示逾期数量
    int overdueCount = m_overdueModel->rowCount();
    emit operationSuccess(QString("共查询到 %1 条逾期未还记录！").arg(overdueCount));
}
