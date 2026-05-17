#ifndef PURCHASES_H
#define PURCHASES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPair>
#include <qdatetime.h>

struct CartItem {
    int qty = 1;
    double purchasePrice;
    double salePrice;
    QDate expiryDate;
};

class Purchases
{
private:
    int id;

    QString invoiceNumber;
    QString type;
    QString date;

    QString supplier;
    double total;

    QString paymentMethod;

public:
    Purchases();

    Purchases(
        int id,
        QString invoiceNumber,
        QString type,
        QString date,
        QString supplier,
        double total,
        QString paymentMethod
        );

    // ================= DATABASE =================
    static int saveInvoice(
        QString invoiceNo,
        QString date,
        QString supplier,
        QString paymentMethod,
        const QMap<int, CartItem> &cart
        );

    static QList<Purchases> getAll();

    static bool deleteInvoice(int invoiceId);

    // ================= GETTERS =================
    int getId() const;

    QString getInvoiceNumber() const;

    QString getType() const;

    QString getDate() const;

    QString getSupplier() const;

    double getTotal() const;

    QString getPaymentMethod() const;

    int getItemsCount() const;

};

#endif