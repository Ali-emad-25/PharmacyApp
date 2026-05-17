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
    bool isReturnMode = false;

private:
    Ui::MainWindow *ui;

    // homewindow *homeCtrl;

    QStandardItemModel *model;
    QStandardItemModel *model2;
    QButtonGroup *payGroup;

    struct CartItem {
        int qty = 1;
        double purchasePrice = 0;
        double salePrice = 0;
        QDate expiryDate;
    };

    QMap<int, CartItem> cart;

    QString invoiceNumber;
    QString invoiceDate;
    bool invoiceStarted = false;

    void updateTotals();
    void updateItemsCount();

    QString getPaymentMethod();
    void generateInvoiceNumber(bool mode);


    bool validateCart(QString &errorMsg);

    void setEditModeUI(bool isEdit);

    int currentInvoiceId = -1;

    QMap<int, QString> returnReasons;
};

#endif