#include "homewindow.h"
#include "ui_mainwindow.h"
#include "action.h"

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


        double sales = Action::getTodaySales();

    ui->homesalevalue->setText(
        QString::number(sales, 'f', 2)
        );

    // ================= TABLE =================
    model = new QStandardItemModel(this);
    ui->hometable->setModel(model);

    ui->hometable->setAlternatingRowColors(true);
    ui->hometable->setShowGrid(false);
    ui->hometable->setFrameShape(QFrame::NoFrame);
    ui->hometable->verticalHeader()->setVisible(false);

    ui->hometable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->hometable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->hometable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->hometable->setFocusPolicy(Qt::NoFocus);

    ui->hometable->setTextElideMode(Qt::ElideRight);

    ui->hometable->verticalHeader()->setDefaultSectionSize(45);
    ui->hometable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    auto header = ui->hometable->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignCenter);

    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);

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
}

void homewindow::loadData()
{
    model->clear();

    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({
        "رقم الفاتورة", "النوع", "التاريخ",
        "الإجمالي", "الدفع", "الإجراءات"
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

        QPushButton *deleteBtn = new QPushButton();
        QPushButton *actionBtn  = new QPushButton();

        deleteBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/trash.svg"));

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

        deleteBtn->setIconSize(QSize(16,16));

        deleteBtn->setFixedSize(28,28);
        actionBtn->setFixedSize(28,28);

        actionBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setCursor(Qt::PointingHandCursor);

        layout->addWidget(actionBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignCenter);

        ui->hometable->setIndexWidget(model->index(row, 5), widget);

        connect(deleteBtn, &QPushButton::clicked, this, [=](){

            auto reply = QMessageBox::question(this,
                                               "تأكيد",
                                               "هل تريد حذف الفاتورة؟",
                                               QMessageBox::Yes | QMessageBox::No
                                               );

            if (reply == QMessageBox::Yes)
            {
                Action::deleteInvoice(invoiceId);
                loadData();
            }
        });

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
}

homewindow::~homewindow()
{
    delete ui;
}