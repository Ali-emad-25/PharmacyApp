#include "medicine.h"
#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

Medicine::Medicine() {}

Medicine::Medicine(int id, QString name, QString category,
                   double sale_price, int min_quantity,QString barcode)
{
    this->id = id;
    this->name = name;
    this->category = category;
    this->sale_price = sale_price;
    this->min_quantity = min_quantity;
    this->barcode = barcode;
}


bool Medicine::add()
{
    QSqlQuery check(Database::instance());
    check.prepare("SELECT id FROM medicines WHERE barcode=?");
    check.addBindValue(barcode);

    if (check.exec() && check.next())
        return false;

    QSqlQuery q(Database::instance());
    q.prepare(R"(
        INSERT INTO medicines
        (name, category, sale_price, min_quantity, barcode)
        VALUES (?, ?, ?, ?, ?)
    )");

    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(sale_price);
    q.addBindValue(min_quantity);
    q.addBindValue(barcode);

    return q.exec();
}


bool Medicine::update()
{
    QSqlQuery check(Database::instance());
    check.prepare("SELECT id FROM medicines WHERE barcode=? AND id != ?");
    check.addBindValue(barcode);
    check.addBindValue(id);

    if (check.exec() && check.next())
        return false;

    QSqlQuery q(Database::instance());
    q.prepare(R"(
        UPDATE medicines SET
        name=?, category=?, sale_price=?,
        min_quantity=?, barcode=?
        WHERE id=?
    )");

    q.addBindValue(name);
    q.addBindValue(category);
    q.addBindValue(sale_price);
    q.addBindValue(min_quantity);
    q.addBindValue(barcode);
    q.addBindValue(id);

    return q.exec();
}


static Medicine toMedicine(QSqlQuery &q)
{
    return Medicine(
        q.value(0).toInt(),
        q.value(1).toString(),
        q.value(2).toString(),
        q.value(3).toDouble(),
        q.value(4).toInt(),
        q.value(5).toString()
        );
}


QList<Medicine> Medicine::getAll()
{
    QList<Medicine> list;
    QSqlQuery q(Database::instance());

    if (!q.exec("SELECT * FROM medicines"))
        return list;

    while (q.next())
        list.append(toMedicine(q));

    return list;
}

Medicine Medicine::getById(int id)
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT *
        FROM medicines
        WHERE id = ?
    )");

    q.addBindValue(id);

    if(q.exec() && q.next())
    {
        return toMedicine(q);
    }

    return Medicine();
}


QList<Medicine> Medicine::search(const QString &text)
{
    QList<Medicine> list;
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT * FROM medicines
        WHERE name LIKE ? OR barcode LIKE ?
    )");

    QString t = text.trimmed();

    q.addBindValue("%" + t + "%");
    q.addBindValue("%" + t + "%");

    if (!q.exec())
        return list;

    while (q.next())
        list.append(toMedicine(q));

    return list;
}


// QString Medicine::getStatus() const
// {
//     QDate exp = QDate::fromString(expiry_date, Qt::ISODate);
//     QDate today = QDate::currentDate();

//     if (quantity == 0) {
//         return "غير متوفر";
//     }
//     else if (exp <= today) {
//         return "منتهي الصلاحية";
//     }
//     else if (quantity <= min_quantity) {
//         return "ناقص";
//     }

//     return "متوفر";
// }

// QList<Medicine> Medicine::filter(FilterType filter)
// {
//     QList<Medicine> list;

//     QSqlQuery q(Database::instance());
//     q.exec("SELECT * FROM medicines");

//     while (q.next())
//     {
//         Medicine m = toMedicine(q);
//         QString status = m.getStatus();

//         if (filter == ALL)
//             list.append(m);

//         else if (filter == AVAILABLE && m.getStatus() == "متوفر")
//             list.append(m);

//         else if (filter == LOW && m.getStatus() == "ناقص")
//             list.append(m);

//         else if (filter == EXPIRED && m.getStatus() == "منتهي الصلاحية")
//             list.append(m);
//     }

//     return list;
// }


QString Medicine::getName() const { return name; }
QString Medicine::getCategory() const { return category; }
QString Medicine::getBarcode() const { return barcode; }
double Medicine::getSalePrice() const { return sale_price; }
int Medicine::getId() const { return id; }
int Medicine::getMinQuantity() const { return min_quantity; }