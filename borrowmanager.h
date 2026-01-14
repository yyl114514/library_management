#ifndef BORROWMANAGER_H
#define BORROWMANAGER_H

#include <QObject>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QTableView>
#include <QComboBox>
#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include "dbmanager.h"
#include "bookmanager.h"
#include "readermanager.h"

class BorrowManager : public QObject
{
    Q_OBJECT
public:
    explicit BorrowManager(BookManager* bookMgr, ReaderManager* readerMgr, QObject *parent = nullptr);
    ~BorrowManager();

    // 创建借还书管理UI组件
    QWidget* createBorrowWidget();
    // 创建数据统计UI组件
    QWidget* createStatWidget();
    // 创建逾期提醒UI组件
    QWidget* createOverdueWidget();
    // 刷新借阅模型数据
    void refreshBorrowModel();

signals:
    void operationSuccess(const QString& msg);
    void operationFailed(const QString& msg);

private slots:
    void onBorrowBook();
    void onReturnBook();
    void onStatisticData();
    void onCheckOverdue();

private:
    // 初始化借阅模型
    void initBorrowModel();
    // 加载图书/读者到下拉框
    void loadComboData();

    QSqlTableModel *m_borrowModel;    // 借阅表模型
    QSqlQueryModel *m_statModel;      // 统计模型
    QSqlQueryModel *m_overdueModel;   // 逾期模型

    BookManager *m_bookMgr;           // 图书管理器
    ReaderManager *m_readerMgr;       // 读者管理器

    // 借还书控件
    QComboBox *m_bookIdCombo;
    QComboBox *m_readerIdCombo;
    QLabel *m_borrowStatusLabel;
    QTableView *m_borrowTableView;

    // 统计和逾期控件
    QTextEdit *m_statTextEdit;
    QTableView *m_overdueTableView;
};

#endif // BORROWMANAGER_H
