#include "purchases.h"
#include "medicine.h"
#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ================= CONSTRUCTORS =================

Purchases::Purchases() {}

Purchases::Purchases(
    int id,
    QString invoiceNumber,
    QString type,
    QString date,
    QString supplier,
    double total,
    QString paymentMethod)
{
    this->id = id;

    this->invoiceNumber = invoiceNumber;
    this->type = type;
    this->date = date;

    this->supplier = supplier;
    this->total = total;

    this->paymentMethod = paymentMethod;
}

// ================= SAVE INVOICE =================

// int Purchases::saveInvoice(
//     QString invoiceNo,
//     QString date,
//     QString supplier,
//     double total,
//     QString paymentMethod,
//     const QMap<int, int> &purchcart)
// {
//     QSqlDatabase db = Database::instance();

//     if(!db.transaction())
//         return -1;

//     QSqlQuery q(db);

//     q.prepare(R"(
//         INSERT INTO purchase_invoices
//         (invoice_number, type, date, supplier, total, payment_method)
//         VALUES (?, 'بيع', ?, ?, ?, ?)
//     )");

//     q.addBindValue(invoiceNo);
//     q.addBindValue(date);
//     q.addBindValue(supplier);
//     q.addBindValue(total);
//     q.addBindValue(paymentMethod);

//     if(!q.exec())
//     {
//         qDebug() << "SQL ERROR:" << q.lastError().text();
//         db.rollback();
//         return -1;
//     }

//     int invoiceId = q.lastInsertId().toInt();

//     for (auto it = purchcart.begin(); it != purchcart.end(); ++it)
//     {
//         int medicineId = it.key();
//         int qty = it.value();

//         Medicine m = Medicine::getById(medicineId);

//         QSqlQuery iq(db);

//         iq.prepare(R"(
//         INSERT INTO purchase_items
//         (purchase_id, medicine_id, quantity, purchase_price, expiry_date, total)
//         VALUES (?, ?, ?, ?, ?)
//     )");

//         iq.addBindValue(invoiceId);
//         iq.addBindValue(medicineId);
//         iq.addBindValue(qty);
//         iq.addBindValue(m.getSalePrice());
//         iq.addBindValue(m.getSalePrice() * qty);

//         if(!iq.exec())
//         {
//             qDebug() << "SQL ERROR invoice_items:" << iq.lastError().text();
//             db.rollback();
//             return -1;
//         }

//         QSqlQuery stock(db);
//         stock.prepare(R"(
//         UPDATE medicines
//         SET quantity = quantity - ?
//         WHERE id = ?
//     )");

//         stock.addBindValue(qty);
//         stock.addBindValue(medicineId);

//         if (!stock.exec())
//         {
//             db.rollback();
//             return -1;
//         }
//     }

//     db.commit();
//     return invoiceId;
// }


// QMap<int, int> Purchases::getInvoiceItems(int invoiceId)
// {
//     QMap<int, int> items;

//     QSqlQuery q(Database::instance());

//     q.prepare(R"(
//         SELECT medicine_id, quantity
//         FROM invoice_items
//         WHERE invoice_id = ?
//     )");

//     q.addBindValue(invoiceId);

//     if(!q.exec())
//         return items;

//     while(q.next())
//     {
//         int id  = q.value(0).toInt();
//         int qty = q.value(1).toInt();

//         items.insert(id, qty);
//     }

//     return items;
// }

// int Purchases::getAlreadyReturnedQty(int invoiceId, int medicineId)
// {
//     QSqlQuery q(Database::instance());

//     q.prepare(R"(
//         SELECT SUM(ii.quantity)
//         FROM invoice_items ii
//         JOIN invoices i ON ii.invoice_id = i.id
//         WHERE i.reference_invoice_id = ?
//         AND ii.medicine_id = ?
//         AND i.type = 'مرتجع'
//     )");

//     q.addBindValue(invoiceId);
//     q.addBindValue(medicineId);

//     if (!q.exec() || !q.next())
//         return 0;

//     return q.value(0).toInt();
// }

// // ================= RETURN =================
// int Purchases::saveReturnInvoice(
//     QString returnInvoiceNo,
//     int originalInvoiceId,
//     QString date,
//     double subtotal,
//     double tax,
//     double total,
//     const QMap<int, QPair<int, QString>> &returnCart)
// {
//     QSqlDatabase db = Database::instance();

//     if (!db.transaction())
//         return -1;

//     QSqlQuery q(db);

//     // ================= 1. حفظ الفاتورة =================
//     q.prepare(R"(
//         INSERT INTO invoices
//         (invoice_number, type, date, subtotal, tax, total, payment_method, reference_invoice_id)
//         VALUES (?, 'مرتجع', ?, ?, ?, ?, 'كاش', ?)
//     )");

//     q.addBindValue(returnInvoiceNo);
//     q.addBindValue(date);
//     q.addBindValue(subtotal);
//     q.addBindValue(tax);
//     q.addBindValue(total);
//     q.addBindValue(originalInvoiceId);

//     if (!q.exec()) {
//         db.rollback();
//         return -1;
//     }

//     int returnInvoiceId = q.lastInsertId().toInt();

//     // ================= 2. حفظ items =================
//     for (auto it = returnCart.begin(); it != returnCart.end(); ++it)
//     {
//         int medicineId = it.key();
//         int qty        = it.value().first;
//         QString reason = it.value().second;

//         Medicine m = Medicine::getById(medicineId);

//         QSqlQuery iq(db);
//         iq.prepare(R"(
//             INSERT INTO invoice_items
//             (invoice_id, medicine_id, quantity, price, total, return_reason)
//             VALUES (?, ?, ?, ?, ?, ?)
//         )");

//         iq.addBindValue(returnInvoiceId);
//         iq.addBindValue(medicineId);
//         iq.addBindValue(qty);
//         iq.addBindValue(m.getSalePrice());
//         iq.addBindValue(m.getSalePrice() * qty);
//         iq.addBindValue(reason);

//         if (!iq.exec()) {
//             db.rollback();
//             return -1;
//         }

//         // ================= 3. stock rollback =================
//         QSqlQuery stock(db);
//         stock.prepare(R"(
//             UPDATE medicines
//             SET quantity = quantity + ?
//             WHERE id = ?
//         )");

//         stock.addBindValue(qty);
//         stock.addBindValue(medicineId);

//         if (!stock.exec()) {
//             db.rollback();
//             return -1;
//         }
//     }

//     db.commit();
//     return returnInvoiceId;
// }

// // ================= DELETE =================

// bool Purchases::deleteInvoice(int invoiceId)
// {
//     QSqlQuery q(Database::instance());

//     q.prepare("DELETE FROM invoices WHERE id=?");
//     q.addBindValue(invoiceId);

//     return q.exec();
// }

// // ================= CONVERT =================

// static Purchases toPurchases(QSqlQuery &q)
// {
//     return Purchases(
//         q.value(0).toInt(),
//         q.value(1).toString(),
//         q.value(2).toString(),
//         q.value(3).toString(),
//         q.value(4).toString(),
//         q.value(5).toDouble(),
//         q.value(6).toString()
//         );
// }

// // ================= GET ALL =================

// QList<Purchases> Purchases::getAll()
// {
//     QList<Purchases> list;

//     QSqlQuery q(Database::instance());

//     q.exec(R"(
//         SELECT
//         id,
//         invoice_number,
//         type,
//         date,
//         subtotal,
//         tax,
//         total,
//         payment_method
//         FROM invoices
//         ORDER BY id DESC
//     )");

//     while(q.next())
//     {
//         list.append(toPurchases(q));
//     }

//     return list;
// }

// // ================= SEARCH =================

// QList<Purchases> Purchases::search(const QString &text)
// {
//     QList<Purchases> list;

//     QSqlQuery q(Database::instance());

//     q.prepare(R"(
//         SELECT * FROM invoices
//         WHERE invoice_number LIKE ?
//         OR type LIKE ?
//         OR payment_method LIKE ?
//     )");

//     QString t = text.trimmed();

//     q.addBindValue("%" + t + "%");
//     q.addBindValue(t + "%");
//     q.addBindValue(t + "%");

//     if(!q.exec())
//         return list;

//     while(q.next())
//     {
//         list.append(toPurchases(q));
//     }

//     return list;
// }


// ================= GETTERS =================

int Purchases::getId() const
{
    return id;
}

QString Purchases::getInvoiceNumber() const
{
    return invoiceNumber;
}

QString Purchases::getType() const
{
    return type;
}

QString Purchases::getDate() const
{
    return date;
}

QString Purchases::getSupplier() const
{
    return supplier;
}

double Purchases::getTotal() const
{
    return total;
}

QString Purchases::getPaymentMethod() const
{
    return paymentMethod;
}