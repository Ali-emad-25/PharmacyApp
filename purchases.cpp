#include "purchases.h"
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

int Purchases::saveInvoice(
    QString invoiceNo,
    QString date,
    QString supplier,
    QString paymentMethod,
    const QMap<int, CartItem> &cart)
{
    QSqlDatabase db = Database::instance();

    if (!db.transaction()) {
        qDebug() << "Transaction Error";
        return -1;
    }

    QSqlQuery q(db);

    // ================= 1. حساب الإجمالي =================
    double totalInvoice = 0;

    for (auto it = cart.begin(); it != cart.end(); ++it) {
        const auto &item = it.value();
        totalInvoice += item.qty * item.purchasePrice;
    }

    // ================= 2. إدخال الفاتورة =================
    q.prepare(R"(
        INSERT INTO purchase_invoices
        (invoice_number, type, date, supplier, total, payment_method)
        VALUES (?, 'شراء', ?, ?, ?, ?)
    )");

    q.addBindValue(invoiceNo);
    q.addBindValue(date);
    q.addBindValue(supplier);
    q.addBindValue(totalInvoice);
    q.addBindValue(paymentMethod);

    if (!q.exec()) {
        qDebug() << "Invoice Error:" << q.lastError().text();
        db.rollback();
        return -1;
    }

    int invoiceId = q.lastInsertId().toInt();

    // ================= 3. إدخال العناصر + تحديث المخزون =================
    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        int medicineId = it.key();
        const auto &item = it.value();

        QSqlQuery iq(db);

        iq.prepare(R"(
            INSERT INTO purchase_items
            (purchase_id, medicine_id, quantity, purchase_price, expiry_date, total)
            VALUES (?, ?, ?, ?, ?, ?)
        )");

        iq.addBindValue(invoiceId);
        iq.addBindValue(medicineId);
        iq.addBindValue(item.qty);
        iq.addBindValue(item.purchasePrice);
        iq.addBindValue(item.expiryDate.toString("yyyy-MM-dd"));
        iq.addBindValue(item.qty * item.purchasePrice);

        if (!iq.exec()) {
            qDebug() << "Item Error:" << iq.lastError().text();
            db.rollback();
            return -1;
        }

        // ================= تحديث المخزون =================
        QSqlQuery stock(db);

        stock.prepare(R"(
            UPDATE medicines
            SET quantity = quantity + ?, sale_price = ?
            WHERE id = ?
        )");

        stock.addBindValue(item.qty);
        stock.addBindValue(item.salePrice);
        stock.addBindValue(medicineId);

        if (!stock.exec()) {
            qDebug() << "Stock Error:" << stock.lastError().text();
            db.rollback();
            return -1;
        }
    }

    // ================= 4. تأكيد العملية =================
    if (!db.commit()) {
        qDebug() << "Commit Error";
        db.rollback();
        return -1;
    }

    return invoiceId;
}

QList<Purchases> Purchases::getAll()
{
    QList<Purchases> list;

    QSqlQuery q(Database::instance());

    q.exec(R"(
        SELECT
            id,
            invoice_number,
            type,
            date,
            supplier,
            total,
            payment_method
        FROM purchase_invoices
        ORDER BY id DESC
    )");

    while(q.next())
    {
        list.append(Purchases(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString(),
            q.value(3).toString(),
            q.value(4).toString(),
            q.value(5).toDouble(),
            q.value(6).toString()
            ));
    }

    return list;
}

bool Purchases::deleteInvoice(int invoiceId)
{
    QSqlQuery q(Database::instance());

    q.prepare("DELETE FROM purchase_invoices WHERE id=?");
    q.addBindValue(invoiceId);

    return q.exec();
}

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