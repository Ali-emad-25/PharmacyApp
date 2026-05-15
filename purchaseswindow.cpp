#include "purchaseswindow.h"
#include "ui_mainwindow.h"

purchaseswindow::purchaseswindow(Ui::MainWindow *ui)
{
    this->ui = ui;
}

purchaseswindow::~purchaseswindow()
{
    delete ui;
}