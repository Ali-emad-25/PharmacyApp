#include "action.h"
#include "medicine.h"
#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ================= CONSTRUCTORS =================

Action::Action() {}

Action::Action(
    int id,
    QString invoiceNumber,
    QString type,
    QString date,
    double subtotal,
    double tax,
    double total,
    QString paymentMethod)
{
    this->id = id;

    this->invoiceNumber = invoiceNumber;
    this->type = type;
    this->date = date;

    this->subtotal = subtotal;
    this->tax = tax;
    this->total = total;

    this->paymentMethod = paymentMethod;
}

// ================= SAVE INVOICE =================

int Action::saveInvoice(
    QString invoiceNo,
    QString date,
    double subtotal,
    double tax,
    double total,
    QString paymentMethod,
    const QMap<int, int> &cart)
{
    QSqlDatabase db = Database::instance();

    if (!db.transaction())
        return -1;

    QSqlQuery q(db);

    q.prepare(R"(
        INSERT INTO invoices
        (invoice_number, type, date, subtotal, tax, total, payment_method)
        VALUES (?, 'بيع', ?, ?, ?, ?, ?)
    )");

    q.addBindValue(invoiceNo);
    q.addBindValue(date);
    q.addBindValue(subtotal);
    q.addBindValue(tax);
    q.addBindValue(total);
    q.addBindValue(paymentMethod);

    if (!q.exec())
    {
        db.rollback();
        return -1;
    }

    int invoiceId = q.lastInsertId().toInt();

    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        int medicineId = it.key();
        int qty = it.value();

        auto batches = deductFEFO(db, medicineId, qty);

        if (batches.isEmpty())
        {
            db.rollback();
            return -1;
        }

        Medicine m = Medicine::getById(medicineId);

        for (auto &b : batches)
        {
            int batchId = b.first;
            int takenQty = b.second;

            QSqlQuery iq(db);
            iq.prepare(R"(
                INSERT INTO invoice_items
                (invoice_id, medicine_id, quantity, purchase_price, sale_price, total, batch_id)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            )");

            iq.addBindValue(invoiceId);
            iq.addBindValue(medicineId);
            iq.addBindValue(takenQty);
            iq.addBindValue(m.getSalePrice());
            iq.addBindValue(m.getSalePrice());
            iq.addBindValue(m.getSalePrice() * takenQty);
            iq.addBindValue(batchId);

            if (!iq.exec())
            {
                db.rollback();
                return -1;
            }
        }
    }

    db.commit();
    return invoiceId;
}

bool Action::returnFEFO(QSqlDatabase &db, int medicineId, int qty)
{
    QSqlQuery q(db);

    q.prepare(R"(
        SELECT id, used_qty
        FROM purchase_items
        WHERE medicine_id = ?
          AND used_qty > 0
        ORDER BY expiry_date DESC, id DESC
    )");

    q.addBindValue(medicineId);

    if (!q.exec())
        return false;

    int remaining = qty;

    while (q.next() && remaining > 0)
    {
        int batchId = q.value(0).toInt();
        int used = q.value(1).toInt();

        int giveBack = qMin(used, remaining);

        QSqlQuery u(db);
        u.prepare(R"(
            UPDATE purchase_items
            SET used_qty = used_qty - ?,
                returned_qty = returned_qty + ?
            WHERE id = ?
        )");

        u.addBindValue(giveBack);
        u.addBindValue(giveBack);
        u.addBindValue(batchId);

        if (!u.exec())
            return false;

        remaining -= giveBack;
    }

    return remaining == 0;
}

QMap<int, int> Action::getInvoiceItems(int invoiceId)
{
    QMap<int, int> items;

    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT medicine_id, quantity
        FROM invoice_items
        WHERE invoice_id = ?
    )");

    q.addBindValue(invoiceId);

    if(!q.exec())
        return items;

    while(q.next())
    {
        int id  = q.value(0).toInt();
        int qty = q.value(1).toInt();

        items.insert(id, qty);
    }

    return items;
}

int Action::getAlreadyReturnedQty(int invoiceId, int medicineId)
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT SUM(ii.quantity)
        FROM invoice_items ii
        JOIN invoices i ON ii.invoice_id = i.id
        WHERE i.reference_invoice_id = ?
        AND ii.medicine_id = ?
        AND i.type = 'مرتجع'
    )");

    q.addBindValue(invoiceId);
    q.addBindValue(medicineId);

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

// ================= RETURN =================
int Action::saveReturnInvoice(
    QString returnInvoiceNo,
    int originalInvoiceId,
    QString date,
    double subtotal,
    double tax,
    double total,
    const QMap<int, QPair<int, QString>> &returnCart)
{
    QSqlDatabase db = Database::instance();

    if (!db.transaction())
        return -1;

    QSqlQuery q(db);

    // 1. header
    q.prepare(R"(
        INSERT INTO invoices
        (invoice_number, type, date, subtotal, tax, total, payment_method, reference_invoice_id)
        VALUES (?, 'مرتجع', ?, ?, ?, ?, 'كاش', ?)
    )");

    q.addBindValue(returnInvoiceNo);
    q.addBindValue(date);
    q.addBindValue(subtotal);
    q.addBindValue(tax);
    q.addBindValue(total);
    q.addBindValue(originalInvoiceId);

    if (!q.exec()) {
        db.rollback();
        return -1;
    }

    int returnInvoiceId = q.lastInsertId().toInt();

    // 2. items
    for (auto it = returnCart.begin(); it != returnCart.end(); ++it)
    {
        int medicineId = it.key();
        int qty = it.value().first;
        QString reason = it.value().second;

        Medicine m = Medicine::getById(medicineId);

        auto batches = returnExactBatches(db, originalInvoiceId, medicineId, qty);

        if (batches.isEmpty()) {
            db.rollback();
            return -1;
        }

        for (auto &b : batches)
        {
            int batchId = b.first;
            int returnedQty = b.second;

            QSqlQuery iq(db);
            iq.prepare(R"(
                INSERT INTO invoice_items
                (invoice_id, medicine_id, quantity, purchase_price, sale_price, total, return_reason, batch_id)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            )");

            iq.addBindValue(returnInvoiceId);
            iq.addBindValue(medicineId);
            iq.addBindValue(returnedQty);
            iq.addBindValue(m.getSalePrice());
            iq.addBindValue(m.getSalePrice());
            iq.addBindValue(m.getSalePrice() * returnedQty);
            iq.addBindValue(reason);
            iq.addBindValue(batchId);

            if (!iq.exec()) {
                db.rollback();
                return -1;
            }
        }
    }

    db.commit();
    return returnInvoiceId;
}

bool Action::canReturn(int invoiceId, int medicineId, int qty)
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT SUM(quantity - returned_qty)
        FROM invoice_items
        WHERE invoice_id = ?
          AND medicine_id = ?
    )");

    q.addBindValue(invoiceId);
    q.addBindValue(medicineId);

    if (!q.exec() || !q.next())
        return false;

    int available = q.value(0).toInt();

    return qty <= available;
}

QList<QPair<int,int>> Action::deductFEFO(QSqlDatabase &db, int medicineId, int qty)
{
    QList<QPair<int,int>> result;

    QSqlQuery q(db);
    q.prepare(R"(
    SELECT id, quantity, used_qty
    FROM purchase_items
    WHERE medicine_id = ?
      AND (quantity - used_qty) > 0
      AND DATE(expiry_date) > DATE('now')
    ORDER BY expiry_date ASC, id ASC
)");

    q.addBindValue(medicineId);

    if (!q.exec())
        return {};

    int remaining = qty;

    while (q.next() && remaining > 0)
    {
        int batchId = q.value(0).toInt();
        int quantity = q.value(1).toInt();
        int used = q.value(2).toInt();

        int available = quantity - used;
        int take = qMin(available, remaining);

        QSqlQuery u(db);
        u.prepare("UPDATE purchase_items SET used_qty = used_qty + ? WHERE id = ?");
        u.addBindValue(take);
        u.addBindValue(batchId);

        if (!u.exec())
            return {};

        result.append(qMakePair(batchId, take));

        remaining -= take;
    }

    if (remaining > 0)
        return {};

    return result;
}

QList<QPair<int,int>> Action::returnExactBatches(
    QSqlDatabase &db,
    int invoiceId,
    int medicineId,
    int qty)
{
    QList<QPair<int,int>> result;

    QSqlQuery q(db);

    q.prepare(R"(
        SELECT id, batch_id, quantity, returned_qty
        FROM invoice_items
        WHERE invoice_id = ?
          AND medicine_id = ?
        ORDER BY id DESC
    )");

    q.addBindValue(invoiceId);
    q.addBindValue(medicineId);

    if (!q.exec())
        return {};

    int remaining = qty;

    while (q.next() && remaining > 0)
    {
        int itemId   = q.value(0).toInt();
        int batchId  = q.value(1).toInt();
        int soldQty  = q.value(2).toInt();
        int returned = q.value(3).toInt();

        int available = soldQty - returned;
        if (available <= 0)
            continue;

        int take = qMin(available, remaining);

        // ✅ 1. رجّع المخزون للباتش
        QSqlQuery u(db);
        u.prepare(R"(
            UPDATE purchase_items
            SET used_qty = used_qty - ?
            WHERE id = ?
        )");

        u.addBindValue(take);
        u.addBindValue(batchId);

        if (!u.exec())
            return {};

        // ✅ 2. حدّث المرتجع في الفاتورة الأصلية
        QSqlQuery u2(db);
        u2.prepare(R"(
            UPDATE invoice_items
            SET returned_qty = returned_qty + ?
            WHERE id = ?
        )");

        u2.addBindValue(take);
        u2.addBindValue(itemId);

        if (!u2.exec())
            return {};

        result.append({batchId, take});

        remaining -= take;
    }

    return (remaining == 0) ? result : QList<QPair<int,int>>{};
}

// ================= DELETE =================

bool Action::deleteInvoice(int invoiceId)
{
    QSqlQuery q(Database::instance());

    q.prepare("DELETE FROM invoices WHERE id=?");
    q.addBindValue(invoiceId);

    return q.exec();
}

// ================= CONVERT =================

static Action toAction(QSqlQuery &q)
{
    return Action(
        q.value(0).toInt(),
        q.value(1).toString(),
        q.value(2).toString(),
        q.value(3).toString(),
        q.value(4).toDouble(),
        q.value(5).toDouble(),
        q.value(6).toDouble(),
        q.value(7).toString()
        );
}

// ================= GET ALL =================

QList<Action> Action::getAll()
{
    QList<Action> list;

    QSqlQuery q(Database::instance());

    q.exec(R"(
        SELECT
        id,
        invoice_number,
        type,
        date,
        subtotal,
        tax,
        total,
        payment_method
        FROM invoices
        ORDER BY id DESC
    )");

    while(q.next())
    {
        list.append(toAction(q));
    }

    return list;
}

// ================= SEARCH =================

QList<Action> Action::search(const QString &text)
{
    QList<Action> list;

    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT * FROM invoices
        WHERE invoice_number LIKE ?
        OR type LIKE ?
        OR payment_method LIKE ?
    )");

    QString t = text.trimmed();

    q.addBindValue("%" + t + "%");
    q.addBindValue(t + "%");
    q.addBindValue(t + "%");

    if(!q.exec())
        return list;

    while(q.next())
    {
        list.append(toAction(q));
    }

    return list;
}

// ================= FILTER =================

QList<Action> Action::filter(FilterType filter)
{
    QList<Action> list;

    QSqlQuery q(Database::instance());
    q.exec("SELECT * FROM invoices");

    while (q.next())
    {
        Action a = toAction(q);

        if (filter == ALL)
            list.append(a);

        else if (filter == SALE && a.getType() == "بيع")
            list.append(a);

        else if (filter == RETURN && a.getType() == "مرتجع")
            list.append(a);
    }

    return list;
}

double Action::getTodaySales()
{
    QSqlQuery q(Database::instance());

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    q.prepare(R"(
        SELECT SUM(total)
        FROM invoices
        WHERE type = 'بيع'
        AND date = ?
    )");

    q.addBindValue(today);

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toDouble();
}

double Action::getTodayReturns()
{
    QSqlQuery q(Database::instance());

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    q.prepare(R"(
        SELECT SUM(total)
        FROM invoices
        WHERE type = 'مرتجع'
        AND date = ?
    )");

    q.addBindValue(today);

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

int Action::getReturnsCounter()
{
    QSqlQuery q(Database::instance());

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    q.prepare(R"(
        SELECT COUNT(*)
        FROM invoices
        WHERE type = 'مرتجع'
        AND date = ?
    )");

    q.addBindValue(today);

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

// ================= GETTERS =================

int Action::getId() const
{
    return id;
}

QString Action::getInvoiceNumber() const
{
    return invoiceNumber;
}

QString Action::getType() const
{
    return type;
}

QString Action::getDate() const
{
    return date;
}

double Action::getSubtotal() const
{
    return subtotal;
}

double Action::getTax() const
{
    return tax;
}

double Action::getTotal() const
{
    return total;
}

QString Action::getPaymentMethod() const
{
    return paymentMethod;
}