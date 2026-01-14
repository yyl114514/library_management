#include "readermanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

ReaderManager::ReaderManager(QObject *parent)
    : QObject(parent)
{
    initReaderModel();
}

ReaderManager::~ReaderManager()
{
    delete m_readerModel;
}

void ReaderManager::initReaderModel()
{
    m_readerModel = new QSqlTableModel(this, DbManager::getInstance().getDatabase());
    m_readerModel->setTable("readers");
    m_readerModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_readerModel->select();

    // 设置列名
    m_readerModel->setHeaderData(0, Qt::Horizontal, "读者ID");
    m_readerModel->setHeaderData(1, Qt::Horizontal, "读者姓名");
    m_readerModel->setHeaderData(2, Qt::Horizontal, "性别");
    m_readerModel->setHeaderData(3, Qt::Horizontal, "年龄");
    m_readerModel->setHeaderData(4, Qt::Horizontal, "手机号");
    m_readerModel->setHeaderData(5, Qt::Horizontal, "注册时间");
}

QWidget* ReaderManager::createReaderWidget()
{
    QWidget *readerWidget = new QWidget;
    QVBoxLayout *readerVLayout = new QVBoxLayout(readerWidget);

    // 读者录入表单
    QGroupBox *readerFormGroup = new QGroupBox("读者信息录入");
    QFormLayout *readerFormLayout = new QFormLayout;
    m_readerNameEdit = new QLineEdit;
    m_genderEdit = new QLineEdit;
    m_ageEdit = new QLineEdit;
    m_ageEdit->setPlaceholderText("请输入数字");
    m_phoneEdit = new QLineEdit;
    readerFormLayout->addRow("读者姓名：", m_readerNameEdit);
    readerFormLayout->addRow("性别：", m_genderEdit);
    readerFormLayout->addRow("年龄：", m_ageEdit);
    readerFormLayout->addRow("手机号：", m_phoneEdit);

    // 读者操作按钮
    QHBoxLayout *readerBtnLayout = new QHBoxLayout;
    QPushButton *addReaderBtn = new QPushButton("添加读者");
    QPushButton *editReaderBtn = new QPushButton("修改选中");
    QPushButton *deleteReaderBtn = new QPushButton("删除选中");
    readerBtnLayout->addWidget(addReaderBtn);
    readerBtnLayout->addWidget(editReaderBtn);
    readerBtnLayout->addWidget(deleteReaderBtn);
    readerBtnLayout->addStretch();

    // 读者查询区域
    QHBoxLayout *readerSearchLayout = new QHBoxLayout;
    m_readerSearchType = new QComboBox;
    m_readerSearchType->addItems({"读者姓名", "手机号"});
    m_readerSearchEdit = new QLineEdit;
    m_readerSearchEdit->setPlaceholderText("输入查询关键词");
    QPushButton *searchReaderBtn = new QPushButton("查询");
    readerSearchLayout->addWidget(new QLabel("查询条件："));
    readerSearchLayout->addWidget(m_readerSearchType);
    readerSearchLayout->addWidget(m_readerSearchEdit);
    readerSearchLayout->addWidget(searchReaderBtn);
    readerSearchLayout->addStretch();

    // 读者表格视图
    m_readerTableView = new QTableView;
    m_readerTableView->setModel(m_readerModel);
    m_readerTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_readerTableView->resizeColumnsToContents();

    // 组装界面
    readerFormGroup->setLayout(readerFormLayout);
    readerVLayout->addWidget(readerFormGroup);
    readerVLayout->addLayout(readerBtnLayout);
    readerVLayout->addLayout(readerSearchLayout);
    readerVLayout->addWidget(m_readerTableView);

    // 绑定槽函数
    connect(addReaderBtn, &QPushButton::clicked, this, &ReaderManager::onAddReader);
    connect(editReaderBtn, &QPushButton::clicked, this, &ReaderManager::onEditReader);
    connect(deleteReaderBtn, &QPushButton::clicked, this, &ReaderManager::onDeleteReader);
    connect(searchReaderBtn, &QPushButton::clicked, this, &ReaderManager::onSearchReader);

    return readerWidget;
}

void ReaderManager::refreshModel()
{
    m_readerModel->submitAll();
    m_readerModel->select();
}

bool ReaderManager::validateReaderInput(const QString& readerName, const QString& gender,
                                        const QString& ageStr, const QString& phone)
{
    if (readerName.isEmpty() || gender.isEmpty() || ageStr.isEmpty() || phone.isEmpty()) {
        emit operationFailed("请填写所有必填字段！");
        return false;
    }

    bool ok;
    int age = ageStr.toInt(&ok);
    if (!ok || age < 0 || age > 150) {
        emit operationFailed("年龄必须是0-150的整数！");
        return false;
    }

    return true;
}

void ReaderManager::onAddReader()
{
    QString readerName = m_readerNameEdit->text().trimmed();
    QString gender = m_genderEdit->text().trimmed();
    QString ageStr = m_ageEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();

    if (!validateReaderInput(readerName, gender, ageStr, phone)) {
        return;
    }

    int age = ageStr.toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO readers (reader_name, gender, age, phone) VALUES (?, ?, ?, ?)");
    query.addBindValue(readerName);
    query.addBindValue(gender);
    query.addBindValue(age);
    query.addBindValue(phone);

    if (query.exec()) {
        // 清空输入框
        m_readerNameEdit->clear();
        m_genderEdit->clear();
        m_ageEdit->clear();
        m_phoneEdit->clear();
        refreshModel();
        emit operationSuccess("读者添加成功！");
    } else {
        emit operationFailed("添加失败：" + query.lastError().text());
    }
}

void ReaderManager::onEditReader()
{
    QModelIndexList selected = m_readerTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        emit operationFailed("请先选中要修改的读者！");
        return;
    }

    int row = selected.first().row();
    int readerId = m_readerModel->data(m_readerModel->index(row, 0)).toInt();

    QString readerName = m_readerNameEdit->text().trimmed();
    QString gender = m_genderEdit->text().trimmed();
    QString ageStr = m_ageEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();

    if (!validateReaderInput(readerName, gender, ageStr, phone)) {
        return;
    }

    int age = ageStr.toInt();

    QSqlQuery query;
    query.prepare("UPDATE readers SET reader_name=?, gender=?, age=?, phone=? WHERE id=?");
    query.addBindValue(readerName);
    query.addBindValue(gender);
    query.addBindValue(age);
    query.addBindValue(phone);
    query.addBindValue(readerId);

    if (query.exec()) {
        refreshModel();
        emit operationSuccess("读者修改成功！");
    } else {
        emit operationFailed("修改失败：" + query.lastError().text());
    }
}

void ReaderManager::onDeleteReader()
{
    QModelIndexList selected = m_readerTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        emit operationFailed("请先选中要删除的读者！");
        return;
    }

    int row = selected.first().row();
    int readerId = m_readerModel->data(m_readerModel->index(row, 0)).toInt();

    if (QMessageBox::question(nullptr, "确认", "确定要删除该读者吗？删除后相关借阅记录也会受影响！",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM readers WHERE id=?");
    query.addBindValue(readerId);

    if (query.exec()) {
        refreshModel();
        emit operationSuccess("读者删除成功！");
    } else {
        emit operationFailed("删除失败：" + query.lastError().text());
    }
}

void ReaderManager::onSearchReader()
{
    QString keyword = m_readerSearchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        m_readerModel->setFilter("");
        refreshModel();
        return;
    }

    QString filter;
    int type = m_readerSearchType->currentIndex();
    if (type == 0) { // 读者姓名
        filter = QString("reader_name LIKE '%%1%'").arg(keyword);
    } else if (type == 1) { // 手机号
        filter = QString("phone LIKE '%%1%'").arg(keyword);
    }

    m_readerModel->setFilter(filter);
    refreshModel();
}
