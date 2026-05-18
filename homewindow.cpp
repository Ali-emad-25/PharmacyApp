#include "homewindow.h"
#include "purchases.h"
#include "ui_mainwindow.h"
#include "action.h"
#include "purchaseitem.h"

homewindow::homewindow(Ui::MainWindow *ui)
{
    this->ui = ui;

    payGroup = new QButtonGroup(this);
    payGroup->addButton(ui->hometablebtnall);
    payGroup->addButton(ui->hometablebtnsale);
    payGroup->addButton(ui->hometablebtnreturn);
    payGroup->setExclusive(true);

    ui->hometablebtnall->setChecked(true);
    ui->hometablebtnall->setCursor(Qt::PointingHandCursor);
    ui->hometablebtnsale->setCursor(Qt::PointingHandCursor);
    ui->hometablebtnreturn->setCursor(Qt::PointingHandCursor);

    // ================= TABLE =================
    model = new QStandardItemModel(this);
    ui->hometable->setModel(model);

    ui->hometable->setAlternatingRowColors(true);
    ui->hometable->verticalHeader()->setVisible(false);
    ui->hometable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->hometable->setTextElideMode(Qt::ElideRight);
    ui->hometable->verticalHeader()->setDefaultSectionSize(45);
    ui->hometable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->hometable->setShowGrid(false);
    ui->hometable->setFocusPolicy(Qt::NoFocus);
    ui->hometable->setFrameShape(QFrame::NoFrame);
    ui->hometable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->hometable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    model2 = new QStandardItemModel(this);
    ui->purchtable->setModel(model2);

    ui->purchtable->setAlternatingRowColors(true);
    ui->purchtable->verticalHeader()->setVisible(false);
    ui->purchtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->purchtable->setTextElideMode(Qt::ElideRight);
    ui->purchtable->verticalHeader()->setDefaultSectionSize(45);
    ui->purchtable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->purchtable->setShowGrid(false);
    ui->purchtable->setFocusPolicy(Qt::NoFocus);
    ui->purchtable->setFrameShape(QFrame::NoFrame);
    ui->purchtable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->purchtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ===== Scroll =====
    ui->hometable->horizontalScrollBar()->setCursor(Qt::PointingHandCursor);
    ui->hometable->verticalScrollBar()->setCursor(Qt::PointingHandCursor);

    // ================= SEARCH =================
    connect(ui->homesearch, &QLineEdit::textChanged, this, [=](const QString &text){
        searchText = text.trimmed();
        currentFilter = Action::ALL;
        ui->hometablebtnall->setChecked(true);
        loadData();
    });

    // ================= FILTERS =================
    connect(ui->hometablebtnall, &QPushButton::clicked, this, [=](){
        searchText.clear();
        currentFilter = Action::ALL;
        loadData();
    });

    connect(ui->hometablebtnsale, &QPushButton::clicked, this, [=](){
        searchText.clear();
        currentFilter = Action::SALE;
        loadData();
    });

    connect(ui->hometablebtnreturn, &QPushButton::clicked, this, [=](){
        searchText.clear();
        currentFilter = Action::RETURN;
        loadData();
    });

    loadData();
    loadDatainvoice();
    getTodayNetSales();
    getTodayReturns();
    updateDashboard();
}

void homewindow::updateDashboard()
{
    int expired = PurchaseItem::countExpiredBatches();
    int lowStock = PurchaseItem::countLowStockMedicines();

    ui->homeexpiredvalue->setText(QString::number(expired));
    ui->homelowvalue->setText(QString::number(lowStock));
}

void homewindow::getTodayReturns()
{
    double returns = Action::getReturnsCounter();
    ui->homereturnvalue->setText(QString::number(returns));
}

void homewindow::getTodayNetSales()
{
    double sales = Action::getTodaySales();
    double returns = Action::getTodayReturns();

    double total = sales - returns;
    ui->homesalevalue->setText(QString::number(total, 'f', 2));
}

void homewindow::loadData()
{
    model->clear();

    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({
        "رقم الفاتورة", "النوع", "التاريخ",
        "الإجمالي", "الدفع", ""
    });

    QList<Action> list;

    if (!searchText.isEmpty())
    {
        list = Action::search(searchText);
    }
    else
    {
        list = Action::filter(currentFilter);
    }

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);

    QColor green(34,197,94);
    QColor yellow(234,179,8);

    QFont normalFont("Segoe UI", 10);
    QFont boldFont("Segoe UI", 10, QFont::Bold);

    for(int row = 0; row < list.size(); row++)
    {
        const Action &a = list[row];

        int invoiceId = a.getId();

        auto *no    = new QStandardItem(a.getInvoiceNumber());
        auto *type  = new QStandardItem(a.getType());
        auto *date  = new QStandardItem(a.getDate());
        auto *total = new QStandardItem(QString::number(a.getTotal()));
        auto *pay   = new QStandardItem(a.getPaymentMethod());
        auto *empty = new QStandardItem("");

        QList<QStandardItem*> items = {
            no, type, date, total, pay, empty
        };

        model->appendRow(items);

        for (int i = 0; i < items.size(); i++)
        {
            QStandardItem *item = items[i];
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(gray);
            item->setFont(normalFont);
        }

        no->setForeground(dark);

        QColor color;

        if (a.getType() == "بيع") color = green;
        else  color = yellow;

        type->setForeground(color);

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);

        // QPushButton *deleteBtn = new QPushButton();
        QPushButton *actionBtn  = new QPushButton();

        // deleteBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/trash.svg"));

        if(a.getType() == "بيع")
        {
            actionBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/refresh3.svg"));
            actionBtn->setIconSize(QSize(16,16));
        }
        else
        {
            actionBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/eye.svg"));
            actionBtn->setIconSize(QSize(18,18));
        }

        // deleteBtn->setIconSize(QSize(16,16));

        // deleteBtn->setFixedSize(28,28);
        actionBtn->setFixedSize(28,28);

        actionBtn->setCursor(Qt::PointingHandCursor);
        // deleteBtn->setCursor(Qt::PointingHandCursor);

        layout->addWidget(actionBtn);
        // layout->addWidget(deleteBtn);
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignCenter);

        ui->hometable->setIndexWidget(model->index(row, 5), widget);

        // connect(deleteBtn, &QPushButton::clicked, this, [=](){

        //     auto reply = QMessageBox::question(this,
        //                                        "تأكيد",
        //                                        "هل تريد حذف الفاتورة؟",
        //                                        QMessageBox::Yes | QMessageBox::No
        //                                        );

        //     if (reply == QMessageBox::Yes)
        //     {
        //         Action::deleteInvoice(invoiceId);
        //         loadData();
        //     }
        // });

        connect(actionBtn, &QPushButton::clicked, this, [=](){

            if(a.getType() == "بيع")
            {
                auto reply = QMessageBox::question(this,
                                                   "تأكيد",
                                                   "هل تريد تحويلها لمرتجع؟",
                                                   QMessageBox::Yes | QMessageBox::No
                                                   );

                if (reply == QMessageBox::Yes)
                {
                    // Action::makeReturn(invoiceId);
                }
            }
            else
            {
                auto reply = QMessageBox::question(this,
                                                   "تأكيد",
                                                   "هل تريد عرض تفاصيل المرتجع؟",
                                                   QMessageBox::Yes | QMessageBox::No
                                                   );

                if (reply == QMessageBox::Yes)
                {

                }
            }
            loadData();
        });
    }
    ui->hometable->viewport()->update();

    auto header = ui->hometable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->hometable->setColumnWidth(2, 100);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->hometable->setColumnWidth(0, 100);
}

void homewindow::loadDatainvoice()
{

    model2->removeRows(0, model2->rowCount());

    model2->setColumnCount(7);

    model2->setHorizontalHeaderLabels({
        "رقم الفاتورة",
        "المورد",
        "النوع",
        "التاريخ",
        "الإجمالي",
        "الدفع",
        ""
    });

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);

    QColor green(34,197,94);
    QColor yellow(234,179,8);

    QFont normalFont("Segoe UI", 10);
    QFont boldFont("Segoe UI", 10, QFont::Bold);

    QList<Purchases> list = Purchases::getAll();

    for(int row = 0; row < list.size(); row++)
    {
        const Purchases &p = list[row];

        auto *no    = new QStandardItem(p.getInvoiceNumber());
        auto *sup   = new QStandardItem(p.getSupplier());
        auto *type  = new QStandardItem(p.getType());
        auto *date  = new QStandardItem(p.getDate());
        auto *total = new QStandardItem(QString::number(p.getTotal()));
        auto *pay   = new QStandardItem(p.getPaymentMethod());
        auto *empty = new QStandardItem("");

        QList<QStandardItem*> items = {
            no, sup, type, date, total, pay, empty
        };

        model2->appendRow(items);

        // ================= تنسيق =================
        for (int i = 0; i < items.size(); i++)
        {
            QStandardItem *item = items[i];
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(gray);
            item->setFont(normalFont);
        }

        no->setForeground(dark);
        total->setForeground(teal);
        total->setFont(boldFont);

        QColor color;

        if (p.getType() == "شراء") color = green;
        else  color = yellow;

        type->setForeground(color);

        // ================= Buttons =================
        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);

        // QPushButton *deleteBtn = new QPushButton();
        QPushButton *actionBtn  = new QPushButton();

        // deleteBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/trash.svg"));

        if(p.getType() == "شراء")
        {
            actionBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/refresh3.svg"));
            actionBtn->setIconSize(QSize(16,16));
        }
        else
        {
            actionBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/eye.svg"));
            actionBtn->setIconSize(QSize(18,18));
        }

        // deleteBtn->setIconSize(QSize(16,16));

        // deleteBtn->setFixedSize(28,28);
        actionBtn->setFixedSize(28,28);

        actionBtn->setCursor(Qt::PointingHandCursor);
        // deleteBtn->setCursor(Qt::PointingHandCursor);

        layout->addWidget(actionBtn);
        // layout->addWidget(deleteBtn);
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignCenter);

        ui->purchtable->setIndexWidget(model2->index(row, 6), widget);

        int invoiceId = p.getId();

        // ================= DELETE =================
        // connect(deleteBtn, &QPushButton::clicked, this, [=](){

        //     if(QMessageBox::question(this,"تأكيد","حذف الفاتورة؟")
        //         == QMessageBox::Yes)
        //     {
        //         Purchases::deleteInvoice(invoiceId);
        //         loadDatainvoice();
        //     }
        // });

        // ================= ACTION =================
        connect(actionBtn, &QPushButton::clicked, this, [=](){

            if(p.getType() == "شراء")
            {
                QMessageBox::information(this,"Return","تحويل لمرتجع");
            }
            else
            {
                QMessageBox::information(this,"View","عرض المرتجع");
            }
        });
    }

    ui->purchtable->viewport()->update();

    auto header = ui->purchtable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->purchtable->setColumnWidth(3, 100);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->purchtable->setColumnWidth(0, 100);
}

homewindow::~homewindow()
{
    delete ui;
}

