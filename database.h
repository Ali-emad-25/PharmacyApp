#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>

class Database
{
public:
    static QSqlDatabase instance();
    static bool init();

private:
    static void createTables();
};

#endif