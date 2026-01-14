#ifndef READERMANAGER_H
#define READERMANAGER_H

#include <QObject>
#include <QSqlTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>
#include <QMessageBox>
#include "dbmanager.h"

class ReaderManager : public QObject
{
    Q_OBJECT
public:
    explicit ReaderManager(QObject *parent = nullptr);
    ~ReaderManager();

    // 创建读者管理UI组件
    QWidget* createReaderWidget();
    // 刷新读者模型数据
    void refreshModel();

signals:
    void operationSuccess(const QString& msg);
    void operationFailed(const QString& msg);

private slots:
    void onAddReader();
    void onEditReader();
    void onDeleteReader();
    void onSearchReader();

private:
    // 初始化读者模型
    void initReaderModel();
    // 校验输入数据
    bool validateReaderInput(const QString& readerName, const QString& gender,
                             const QString& ageStr, const QString& phone);

    QSqlTableModel *m_readerModel;  // 读者表模型
    QTableView *m_readerTableView;  // 读者表格视图

    // 读者录入/查询控件
    QLineEdit *m_readerNameEdit;
    QLineEdit *m_genderEdit;
    QLineEdit *m_ageEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_readerSearchEdit;
    QComboBox *m_readerSearchType;
};

#endif // READERMANAGER_H
