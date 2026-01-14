#include "bookmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

BookManager::BookManager(QObject *parent)
    : QObject(parent)
{
    initBookModel();
}

BookManager::~BookManager()
{
    delete m_bookModel;
}

void BookManager::initBookModel()
{
    m_bookModel = new QSqlTableModel(this, DbManager::getInstance().getDatabase());
    m_bookModel->setTable("books");
    m_bookModel->setEditStrategy(QSqlTableModel::OnManualSubmit); // 手动提交修改
    m_bookModel->select();

    // 设置列名
    m_bookModel->setHeaderData(0, Qt::Horizontal, "图书ID");
    m_bookModel->setHeaderData(1, Qt::Horizontal, "图书名称");
    m_bookModel->setHeaderData(2, Qt::Horizontal, "作者");
    m_bookModel->setHeaderData(3, Qt::Horizontal, "出版社");
    m_bookModel->setHeaderData(4, Qt::Horizontal, "ISBN");
    m_bookModel->setHeaderData(5, Qt::Horizontal, "库存");
    m_bookModel->setHeaderData(6, Qt::Horizontal, "添加时间");
}

QWidget* BookManager::createBookWidget()
{
    QWidget *bookWidget = new QWidget;
    QVBoxLayout *bookVLayout = new QVBoxLayout(bookWidget);

    // 图书录入表单
    QGroupBox *bookFormGroup = new QGroupBox("图书信息录入");
    QFormLayout *bookFormLayout = new QFormLayout;
    m_bookNameEdit = new QLineEdit;
    m_authorEdit = new QLineEdit;
    m_publisherEdit = new QLineEdit;
    m_isbnEdit = new QLineEdit;
    m_stockEdit = new QLineEdit;
    m_stockEdit->setPlaceholderText("请输入数字");
    bookFormLayout->addRow("图书名称：", m_bookNameEdit);
    bookFormLayout->addRow("作者：", m_authorEdit);
    bookFormLayout->addRow("出版社：", m_publisherEdit);
    bookFormLayout->addRow("ISBN：", m_isbnEdit);
    bookFormLayout->addRow("库存：", m_stockEdit);

    // 图书操作按钮
    QHBoxLayout *bookBtnLayout = new QHBoxLayout;
    QPushButton *addBookBtn = new QPushButton("添加图书");
    QPushButton *editBookBtn = new QPushButton("修改选中");
    QPushButton *deleteBookBtn = new QPushButton("删除选中");
    bookBtnLayout->addWidget(addBookBtn);
    bookBtnLayout->addWidget(editBookBtn);
    bookBtnLayout->addWidget(deleteBookBtn);
    bookBtnLayout->addStretch();

    // 图书查询区域
    QHBoxLayout *bookSearchLayout = new QHBoxLayout;
    m_bookSearchType = new QComboBox;
    m_bookSearchType->addItems({"图书名称", "作者", "ISBN"});
    m_bookSearchEdit = new QLineEdit;
    m_bookSearchEdit->setPlaceholderText("输入查询关键词");
    QPushButton *searchBookBtn = new QPushButton("查询");
    bookSearchLayout->addWidget(new QLabel("查询条件："));
    bookSearchLayout->addWidget(m_bookSearchType);
    bookSearchLayout->addWidget(m_bookSearchEdit);
    bookSearchLayout->addWidget(searchBookBtn);
    bookSearchLayout->addStretch();

    // 图书表格视图
    m_bookTableView = new QTableView;
    m_bookTableView->setModel(m_bookModel);
    m_bookTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookTableView->resizeColumnsToContents();

    // 组装界面
    bookFormGroup->setLayout(bookFormLayout);
    bookVLayout->addWidget(bookFormGroup);
    bookVLayout->addLayout(bookBtnLayout);
    bookVLayout->addLayout(bookSearchLayout);
    bookVLayout->addWidget(m_bookTableView);

    // 绑定槽函数
    connect(addBookBtn, &QPushButton::clicked, this, &BookManager::onAddBook);
    connect(editBookBtn, &QPushButton::clicked, this, &BookManager::onEditBook);
    connect(deleteBookBtn, &QPushButton::clicked, this, &BookManager::onDeleteBook);
    connect(searchBookBtn, &QPushButton::clicked, this, &BookManager::onSearchBook);

    return bookWidget;
}

void BookManager::refreshModel()
{
    m_bookModel->submitAll(); // 提交修改
    m_bookModel->select();    // 重新查询数据
}

bool BookManager::validateBookInput(const QString& bookName, const QString& author,
                                    const QString& publisher, const QString& isbn, const QString& stockStr)
{
    if (bookName.isEmpty() || author.isEmpty() || publisher.isEmpty() || isbn.isEmpty() || stockStr.isEmpty()) {
        emit operationFailed("请填写所有必填字段！");
        return false;
    }

    bool ok;
    int stock = stockStr.toInt(&ok);
    if (!ok || stock < 0) {
        emit operationFailed("库存必须是非负整数！");
        return false;
    }

    return true;
}

void BookManager::onAddBook()
{
    QString bookName = m_bookNameEdit->text().trimmed();
    QString author = m_authorEdit->text().trimmed();
    QString publisher = m_publisherEdit->text().trimmed();
    QString isbn = m_isbnEdit->text().trimmed();
    QString stockStr = m_stockEdit->text().trimmed();

    if (!validateBookInput(bookName, author, publisher, isbn, stockStr)) {
        return;
    }

    int stock = stockStr.toInt();

    // 插入数据
    QSqlQuery query;
    query.prepare("INSERT INTO books (book_name, author, publisher, isbn, stock) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(bookName);
    query.addBindValue(author);
    query.addBindValue(publisher);
    query.addBindValue(isbn);
    query.addBindValue(stock);

    if (query.exec()) {
        // 清空输入框
        m_bookNameEdit->clear();
        m_authorEdit->clear();
        m_publisherEdit->clear();
        m_isbnEdit->clear();
        m_stockEdit->clear();
        refreshModel();
        emit operationSuccess("图书添加成功！");
    } else {
        emit operationFailed("添加失败：" + query.lastError().text());
    }
}

void BookManager::onEditBook()
{
    // 获取选中行
    QModelIndexList selected = m_bookTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        emit operationFailed("请先选中要修改的图书！");
        return;
    }

    int row = selected.first().row();
    int bookId = m_bookModel->data(m_bookModel->index(row, 0)).toInt();

    // 获取输入值
    QString bookName = m_bookNameEdit->text().trimmed();
    QString author = m_authorEdit->text().trimmed();
    QString publisher = m_publisherEdit->text().trimmed();
    QString isbn = m_isbnEdit->text().trimmed();
    QString stockStr = m_stockEdit->text().trimmed();

    if (!validateBookInput(bookName, author, publisher, isbn, stockStr)) {
        return;
    }

    int stock = stockStr.toInt();

    // 更新数据
    QSqlQuery query;
    query.prepare("UPDATE books SET book_name=?, author=?, publisher=?, isbn=?, stock=? WHERE id=?");
    query.addBindValue(bookName);
    query.addBindValue(author);
    query.addBindValue(publisher);
    query.addBindValue(isbn);
    query.addBindValue(stock);
    query.addBindValue(bookId);

    if (query.exec()) {
        refreshModel();
        emit operationSuccess("图书修改成功！");
    } else {
        emit operationFailed("修改失败：" + query.lastError().text());
    }
}

void BookManager::onDeleteBook()
{
    QModelIndexList selected = m_bookTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        emit operationFailed("请先选中要删除的图书！");
        return;
    }

    int row = selected.first().row();
    int bookId = m_bookModel->data(m_bookModel->index(row, 0)).toInt();

    // 确认删除
    if (QMessageBox::question(nullptr, "确认", "确定要删除该图书吗？删除后相关借阅记录也会受影响！",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM books WHERE id=?");
    query.addBindValue(bookId);

    if (query.exec()) {
        refreshModel();
        emit operationSuccess("图书删除成功！");
    } else {
        emit operationFailed("删除失败：" + query.lastError().text());
    }
}

void BookManager::onSearchBook()
{
    QString keyword = m_bookSearchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        m_bookModel->setFilter(""); // 清空筛选
        refreshModel();
        return;
    }

    // 根据选择的查询条件构建筛选语句
    QString filter;
    int type = m_bookSearchType->currentIndex();
    if (type == 0) { // 图书名称
        filter = QString("book_name LIKE '%%1%'").arg(keyword);
    } else if (type == 1) { // 作者
        filter = QString("author LIKE '%%1%'").arg(keyword);
    } else if (type == 2) { // ISBN
        filter = QString("isbn LIKE '%%1%'").arg(keyword);
    }

    m_bookModel->setFilter(filter);
    refreshModel();
}
