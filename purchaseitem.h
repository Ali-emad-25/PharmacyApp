#ifndef PURCHASEITEM_H
#define PURCHASEITEM_H

#include <QString>
#include <QList>

class PurchaseItem
{
private:
    int id;
    int purchaseId;
    int medicineId;

    QString medicineName;

    int quantity;
    int returnedQty;
    int usedQty;

    double purchasePrice;
    QString expiryDate;

    double total;
    QString invoiceNumber;

public:
    PurchaseItem();

    PurchaseItem(int id,
                 int purchaseId,
                 int medicineId,
                 QString medicineName,
                 int quantity,
                 int returnedQty,
                 double purchasePrice,
                 QString expiryDate,
                 double total,
                 QString invoiceNumber,
                 int usedQty);

    // DB
    static QList<PurchaseItem> getAll();
    static QList<PurchaseItem> getByMedicine(int medicineId);
    static bool deleteItem(int id);

    // GETTERS
    int getId() const;
    int getPurchaseId() const;
    int getMedicineId() const;

    QString getMedicineName() const;

    int getQuantity() const;
    int getReturnedQty() const;
    int getUsedQty() const;

    double getPurchasePrice() const;
    QString getExpiryDate() const;

    double getTotal() const;

    QString getInvoiceNumber() const;

    int getAvailableQty() const;
};

#endif