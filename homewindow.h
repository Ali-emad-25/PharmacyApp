#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include "action.h"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QButtonGroup>
#include <QMessageBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QShortcut>
#include <QScrollBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class homewindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit homewindow(Ui::MainWindow *ui);
    ~homewindow();

    Action::FilterType currentFilter = Action::ALL;

    QString searchText;

    void loadData();
    void loadDatainvoice();
    void getTodayNetSales();
    void getTodayReturns();
    void updateDashboard();

private:
    Ui::MainWindow *ui;

    QStandardItemModel *model;
    QStandardItemModel *model2;
    QButtonGroup *payGroup;
    QButtonGroup *payGroup2;
    QButtonGroup *filterGroup;

    void updateStats();

};

#endif