#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>


#define BORROW_LIMIT_DAYS 30// 借阅期限（天）

class DbManager
{
public:
    static DbManager& getInstance();
    ~DbManager();
    QSqlDatabase& getDatabase();
    bool initDatabase();
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    DbManager();
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
