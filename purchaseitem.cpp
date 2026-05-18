#include "purchaseitem.h"
#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PurchaseItem::PurchaseItem() {}

PurchaseItem::PurchaseItem(int id,
                           int purchaseId,
                           int medicineId,
                           QString medicineName,
                           int quantity,
                           int returnedQty,
                           double purchasePrice,
                           QString expiryDate,
                           double total,
                           QString invoiceNumber,
                           int usedQty)
{
    this->id = id;
    this->purchaseId = purchaseId;
    this->medicineId = medicineId;

    this->medicineName = medicineName;

    this->quantity = quantity;
    this->returnedQty = returnedQty;
    this->usedQty = usedQty;

    this->purchasePrice = purchasePrice;
    this->expiryDate = expiryDate;

    this->total = total;
    this->invoiceNumber = invoiceNumber;
}


QList<PurchaseItem> PurchaseItem::getAll()
{
    QList<PurchaseItem> list;

    QSqlQuery q(Database::instance());

    q.exec(R"(
        SELECT
            pi.id,
            pi.purchase_id,
            pi.medicine_id,
            m.name,
            pi.quantity,
            pi.returned_qty,
            pi.purchase_price,
            pi.expiry_date,
            pi.total,
            p.invoice_number,
            pi.used_qty
        FROM purchase_items pi
        JOIN medicines m ON m.id = pi.medicine_id
        JOIN purchase_invoices p ON p.id = pi.purchase_id
        ORDER BY m.name ASC, date(pi.expiry_date) ASC
    )");

    while (q.next())
    {
        list.append(PurchaseItem(
            q.value(0).toInt(),
            q.value(1).toInt(),
            q.value(2).toInt(),
            q.value(3).toString(),
            q.value(4).toInt(),
            q.value(5).toInt(),
            q.value(6).toDouble(),
            q.value(7).toString(),
            q.value(8).toDouble(),
            q.value(9).toString(),
            q.value(10).toInt()
            ));
    }

    return list;
}

QList<PurchaseItem> PurchaseItem::filter(BatchFilter filter)
{
    QList<PurchaseItem> list;

    QSqlQuery q(Database::instance());

    QString query;

    if (filter == ALL)
    {
        query = R"(
            SELECT
                pi.id,
                pi.purchase_id,
                pi.medicine_id,
                m.name,
                pi.quantity,
                pi.returned_qty,
                pi.purchase_price,
                pi.expiry_date,
                pi.total,
                p.invoice_number,
                pi.used_qty
            FROM purchase_items pi
            JOIN medicines m ON m.id = pi.medicine_id
            JOIN purchase_invoices p ON p.id = pi.purchase_id
        )";
    }

    else if (filter == LOW_STOCK)
    {
        query = R"(
            SELECT
                pi.id,
                pi.purchase_id,
                pi.medicine_id,
                m.name,
                pi.quantity,
                pi.returned_qty,
                pi.purchase_price,
                pi.expiry_date,
                pi.total,
                p.invoice_number,
                pi.used_qty
            FROM purchase_items pi
            JOIN medicines m ON m.id = pi.medicine_id
            JOIN purchase_invoices p ON p.id = pi.purchase_id
            WHERE (pi.quantity - pi.used_qty - pi.returned_qty) <= m.min_quantity
              AND DATE(pi.expiry_date) > DATE('now')
        )";
    }

    else if (filter == EXPIRED)
    {
        query = R"(
            SELECT
                pi.id,
                pi.purchase_id,
                pi.medicine_id,
                m.name,
                pi.quantity,
                pi.returned_qty,
                pi.purchase_price,
                pi.expiry_date,
                pi.total,
                p.invoice_number,
                pi.used_qty
            FROM purchase_items pi
            JOIN medicines m ON m.id = pi.medicine_id
            JOIN purchase_invoices p ON p.id = pi.purchase_id
            WHERE DATE(pi.expiry_date) <= DATE('now')
        )";
    }

    if (!q.exec(query))
    {
        qDebug() << "Batch Filter Error:" << q.lastError().text();
        return {};
    }

    while (q.next())
    {
        list.append(PurchaseItem(
            q.value(0).toInt(),
            q.value(1).toInt(),
            q.value(2).toInt(),
            q.value(3).toString(),
            q.value(4).toInt(),
            q.value(5).toInt(),
            q.value(6).toDouble(),
            q.value(7).toString(),
            q.value(8).toDouble(),
            q.value(9).toString(),
            q.value(10).toInt()
            ));
    }

    return list;
}

int PurchaseItem::countLowStockMedicines()
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT COUNT(DISTINCT pi.medicine_id)
        FROM purchase_items pi
        JOIN medicines m ON m.id = pi.medicine_id
        WHERE (pi.quantity - pi.used_qty - pi.returned_qty) <= m.min_quantity
          AND DATE(pi.expiry_date) > DATE('now')
    )");

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

int PurchaseItem::countExpiredBatches()
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT COUNT(*)
        FROM purchase_items
        WHERE DATE(expiry_date) <= DATE('now')
    )");

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

int PurchaseItem::getAvailableQtyByMedicine(int medicineId)
{
    QSqlQuery q(Database::instance());

    q.prepare(R"(
        SELECT SUM(quantity - used_qty)
        FROM purchase_items
        WHERE medicine_id = ?
          AND DATE(expiry_date) > DATE('now')
    )");

    q.addBindValue(medicineId);

    if (!q.exec() || !q.next())
        return 0;

    return q.value(0).toInt();
}

int PurchaseItem::getAvailableQty() const
{
    return quantity - usedQty - returnedQty;
}

bool PurchaseItem::deleteItem(int id)
{
    QSqlQuery q(Database::instance());

    q.prepare("DELETE FROM purchase_items WHERE id=?");
    q.addBindValue(id);

    return q.exec();
}

// ================= GETTERS =================

int PurchaseItem::getId() const { return id; }

int PurchaseItem::getPurchaseId() const { return purchaseId; }

int PurchaseItem::getMedicineId() const { return medicineId; }

QString PurchaseItem::getMedicineName() const { return medicineName; }

int PurchaseItem::getQuantity() const { return quantity; }

int PurchaseItem::getReturnedQty() const { return returnedQty; }

double PurchaseItem::getPurchasePrice() const { return purchasePrice; }

QString PurchaseItem::getExpiryDate() const { return expiryDate; }

double PurchaseItem::getTotal() const { return total; }

QString PurchaseItem::getInvoiceNumber() const
{
    return invoiceNumber;
}
int PurchaseItem::getUsedQty() const { return usedQty; }