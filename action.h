#ifndef ACTION_H
#define ACTION_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPair>
#include <qsqldatabase.h>


class Action
{
private:
    int id;

    QString invoiceNumber;
    QString type;
    QString date;

    double subtotal;
    double tax;
    double total;

    QString paymentMethod;

public:
    Action();

    Action(
        int id,
        QString invoiceNumber,
        QString type,
        QString date,
        double subtotal,
        double tax,
        double total,
        QString paymentMethod
        );

    // ================= DATABASE =================
    static int saveInvoice(
        QString invoiceNo,
        QString date,
        double subtotal,
        double tax,
        double total,
        QString paymentMethod,
        const QMap<int, int> &cart
        );

    static int saveReturnInvoice(
        QString returnInvoiceNo,
        int originalInvoiceId,
        QString date,
        double subtotal,
        double tax,
        double total,
        const QMap<int, QPair<int, QString>> &returnCart
        );

    static bool deleteInvoice(int invoiceId);

    static QList<Action> getAll();

    static QList<Action> search(const QString &text);

    enum FilterType {
        ALL,
        SALE,
        RETURN
    };

    static QList<Action> filter(FilterType filter);

    static double getTodaySales();

    static bool returnFEFO(QSqlDatabase &db, int medicineId, int qty);

    static int getAvailableStock(QSqlDatabase &db, int medicineId);

    static QList<QPair<int,int>> returnExactBatches(QSqlDatabase &db,
                                                    int invoiceId,
                                                    int medicineId,
                                                    int qty);

    static QList<QPair<int,int>> deductFEFO(QSqlDatabase &db, int medicineId, int qty);

    static bool canReturn(int invoiceId, int medicineId, int qty);

    // ================= GETTERS =================
    int getId() const;

    QString getInvoiceNumber() const;

    QString getType() const;

    QString getDate() const;

    double getSubtotal() const;

    double getTax() const;

    double getTotal() const;

    QString getPaymentMethod() const;

    static QMap<int, int> getInvoiceItems(int invoiceId);

    static int getAlreadyReturnedQty(int invoiceId, int medicineId);
};

#endif