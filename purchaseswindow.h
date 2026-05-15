#ifndef PURCHASESWINDOW_H
#define PURCHASESWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class purchaseswindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit purchaseswindow(Ui::MainWindow *ui);
    ~purchaseswindow();

private:
    Ui::MainWindow *ui;
};

#endif