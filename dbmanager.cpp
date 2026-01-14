#include "dbmanager.h"

DbManager::DbManager()
{
    // 连接SQLite数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("library.db"); // 数据库文件保存在程序运行目录

    // 打开数据库
    if (!m_db.open()) {
        qDebug() << "数据库打开失败：" << m_db.lastError().text();
    }
}

DbManager::~DbManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DbManager& DbManager::getInstance()
{
    static DbManager instance; // 静态局部变量，保证单例
    return instance;
}

QSqlDatabase& DbManager::getDatabase()
{
    return m_db;
}

bool DbManager::initDatabase()
{
    if (!m_db.isOpen()) {
        return false;
    }

    // 创建图书表
    QSqlQuery bookQuery;
    QString bookSql = R"(
        CREATE TABLE IF NOT EXISTS books (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_name TEXT NOT NULL,
            author TEXT NOT NULL,
            publisher TEXT NOT NULL,
            isbn TEXT UNIQUE NOT NULL,
            stock INTEGER NOT NULL DEFAULT 1,
            add_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    if (!bookQuery.exec(bookSql)) {
        qDebug() << "创建图书表失败：" << bookQuery.lastError().text();
        return false;
    }

    // 创建读者表
    QSqlQuery readerQuery;
    QString readerSql = R"(
        CREATE TABLE IF NOT EXISTS readers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            reader_name TEXT NOT NULL,
            gender TEXT NOT NULL,
            age INTEGER NOT NULL,
            phone TEXT UNIQUE NOT NULL,
            register_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    if (!readerQuery.exec(readerSql)) {
        qDebug() << "创建读者表失败：" << readerQuery.lastError().text();
        return false;
    }

    // 创建借阅表
    QSqlQuery borrowQuery;
    QString borrowSql = R"(
        CREATE TABLE IF NOT EXISTS borrows (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_id INTEGER NOT NULL,
            reader_id INTEGER NOT NULL,
            borrow_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            return_time DATETIME,
            status INTEGER DEFAULT 0, -- 0:未还 1:已还
            FOREIGN KEY(book_id) REFERENCES books(id),
            FOREIGN KEY(reader_id) REFERENCES readers(id)
        )
    )";
    if (!borrowQuery.exec(borrowSql)) {
        qDebug() << "创建借阅表失败：" << borrowQuery.lastError().text();
        return false;
    }

    return true;
}

bool DbManager::beginTransaction()
{
    return m_db.transaction();
}

bool DbManager::commitTransaction()
{
    return m_db.commit();
}

bool DbManager::rollbackTransaction()
{
    return m_db.rollback();
}
