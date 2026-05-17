#ifndef PURCHASESWINDOW_H
#define PURCHASESWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QButtonGroup>
#include <QCompleter>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDate>
#include <qcombobox.h>
#include <QDateEdit>

#include "purchases.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class purchaseswindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit purchaseswindow(Ui::MainWindow *ui);
    ~purchaseswindow();

    void refreshCompleter(bool mode);
    void loadData();
    void loadReturnData();
    void loadDatainvoice();
    bool isReturnMode = false;

    QMap<int, CartItem> cart;

private:
    Ui::MainWindow *ui;

    // homewindow *homeCtrl;

    QStandardItemModel *model;
    QStandardItemModel *model2;
    QButtonGroup *payGroup;

    QString invoiceNumber;
    bool invoiceStarted = false;

    int currentInvoiceId = -1;

    QMap<int, QString> returnReasons;

    void updateTotals();
    void updateItemsCount();

    QString getPaymentMethod();
    void generateInvoiceNumber(bool mode);


    bool validateCart(QString &errorMsg);

    void setEditModeUI(bool isEdit);
};

#endif