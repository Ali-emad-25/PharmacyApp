#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "medicine.h"
#include "actionwindow.h"
#include "homewindow.h"
#include "purchaseswindow.h"

#include <QMainWindow>
#include <QButtonGroup>
#include <QSqlQuery>
#include <QSqlError>
#include <QShortcut>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDate>
#include <QScrollBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_midsave_clicked();
    void on_midcancel_clicked();

private:
    Ui::MainWindow *ui;
    actionwindow *actionCtrl;
    homewindow *homeCtrl;
    purchaseswindow *purchasesCtrl;
    QButtonGroup *navGroup;
    QStandardItemModel *model;
    QStandardItemModel *model2;
    QHash<int, Medicine> medicinesMap;

    void loadData();
    void loadDataStock();
    void clearForm();
    void setEditModeUI(bool isEdit);
    bool isValidEAN13(const QString &code);
    void updateStats();

    int currentId = 0;

    QString searchText;

    Medicine::FilterType currentFilter = Medicine::ALL;
};
#endif