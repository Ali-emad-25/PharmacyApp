#ifndef ACTIONWINDOW_H
#define ACTIONWINDOW_H

#include "homewindow.h"

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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class actionwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit actionwindow(Ui::MainWindow *ui, homewindow *homeCtrl);
    ~actionwindow();

    void refreshCompleter(bool mode);
    void loadData();
    void loadReturnData();
    bool isReturnMode = false;


private:
    Ui::MainWindow *ui;

    homewindow *homeCtrl;

    QStandardItemModel *model;
    QStandardItemModel *model2;
    QButtonGroup *payGroup;

    QMap<int, int> cart;

    QString invoiceNumber;
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

