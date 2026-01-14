#ifndef BOOKMANAGER_H
#define BOOKMANAGER_H

#include <QObject>
#include <QSqlTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>
#include <QMessageBox>
#include "dbmanager.h"

class BookManager : public QObject
{
    Q_OBJECT
public:
    explicit BookManager(QObject *parent = nullptr);
    ~BookManager();

    // 创建图书管理UI组件
    QWidget* createBookWidget();
    // 刷新图书模型数据
    void refreshModel();

signals:
    void operationSuccess(const QString& msg);
    void operationFailed(const QString& msg);

private slots:
    void onAddBook();
    void onEditBook();
    void onDeleteBook();
    void onSearchBook();

private:
    // 初始化图书模型
    void initBookModel();
    // 校验输入数据
    bool validateBookInput(const QString& bookName, const QString& author,
                           const QString& publisher, const QString& isbn, const QString& stockStr);

    QSqlTableModel *m_bookModel;    // 图书表模型
    QTableView *m_bookTableView;    // 图书表格视图

    // 图书录入/查询控件
    QLineEdit *m_bookNameEdit;
    QLineEdit *m_authorEdit;
    QLineEdit *m_publisherEdit;
    QLineEdit *m_isbnEdit;
    QLineEdit *m_stockEdit;
    QLineEdit *m_bookSearchEdit;
    QComboBox *m_bookSearchType;
};

#endif // BOOKMANAGER_H
