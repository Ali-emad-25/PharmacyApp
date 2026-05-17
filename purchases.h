#ifndef PURCHASES_H
#define PURCHASES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPair>


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
        double total,
        QString paymentMethod,
        const QMap<int, int> &cart
        );

    static int saveReturnInvoice(
        QString returnInvoiceNo,
        int originalInvoiceId,
        QString date,
        QString supplier,
        double total,
        const QMap<int, QPair<int, QString>> &returnCart
        );

    static bool deleteInvoice(int invoiceId);

    static QList<Purchases> getAll();

    static QList<Purchases> search(const QString &text);

    // ================= GETTERS =================
    int getId() const;

    QString getInvoiceNumber() const;

    QString getType() const;

    QString getDate() const;

    QString getSupplier() const;

    double getTotal() const;

    QString getPaymentMethod() const;

    static QMap<int, int> getInvoiceItems(int invoiceId);

    static int getAlreadyReturnedQty(int invoiceId, int medicineId);
};

#endif