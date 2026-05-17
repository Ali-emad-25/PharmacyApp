#include "purchaseswindow.h"
#include "database.h"
#include "medicine.h"
#include "ui_mainwindow.h"

#include "QcalendarWidget"


purchaseswindow::purchaseswindow(Ui::MainWindow *ui)
{
    this->ui = ui;

    ui->purchvaluedate->setText(QDate::currentDate().toString("yyyy-MM-dd"));

    ui->purchbtnmode->setCursor(Qt::PointingHandCursor);
    ui->purchbtncash->setCursor(Qt::PointingHandCursor);
    ui->purchbtnvisa->setCursor(Qt::PointingHandCursor);
    ui->purchbtndone->setCursor(Qt::PointingHandCursor);
    ui->purchbtncancel->setCursor(Qt::PointingHandCursor);

    // ================= PAYMENT =================
    payGroup = new QButtonGroup(this);
    payGroup->addButton(ui->purchbtncash);
    payGroup->addButton(ui->purchbtnvisa);
    payGroup->setExclusive(true);

    ui->purchbtncash->setChecked(true);

    // ================= TABLE =================
    model = new QStandardItemModel(this);
    ui->purchcart->setModel(model);

    ui->purchcart->verticalHeader()->setVisible(false);
    ui->purchcart->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->purchcart->setSelectionMode(QAbstractItemView::NoSelection);
    ui->purchcart->setTextElideMode(Qt::ElideNone);
    ui->purchcart->verticalHeader()->setDefaultSectionSize(45);
    ui->purchcart->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->purchcart->setShowGrid(false);
    ui->purchcart->setFocusPolicy(Qt::NoFocus);
    ui->purchcart->setFrameShape(QFrame::NoFrame);
    ui->purchcart->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);


    connect(ui->purchsearch, &QLineEdit::returnPressed,
            this, [this, ui]() {

                QString key = ui->purchsearch->text().trimmed();

                if (key.isEmpty()) {
                    QMessageBox::warning(this, "خطأ", "من فضلك أدخل اسم الدواء او الباركود");
                    return;
                }

                // ================= SEARCH MEDICINE =================
                QList<Medicine> result = Medicine::search(key);

                if(result.isEmpty()) {
                    QMessageBox::warning(this, "خطأ", "لا يوجد دواء مطابق لهذا البحث");
                    return;
                }

                Medicine m = result.first();
                int id = m.getId();

                // ================= START INVOICE =================
                if(cart.isEmpty())
                    generateInvoiceNumber(false);

                if (!cart.contains(id))
                {
                    CartItem item;
                    item.qty = 1;

                    Medicine m = Medicine::getById(id);
                    item.purchasePrice = 0;
                    item.salePrice = m.getSalePrice(); // مهم

                    cart.insert(id, item);
                }

                // ================= UPDATE UI =================
                loadData();           // load purchase cart
                updateTotals();       // اجمالي الفاتورة
                updateItemsCount();   // عدد العناصر

                ui->purchsearch->clear();
            });

    connect(ui->purchbtnmode, &QPushButton::toggled, this, [=](bool checked) {
        cart.clear();
        returnReasons.clear();
        invoiceStarted = false;
        invoiceNumber.clear();
        ui->purchvalueinv->setText("0");
        ui->purchvaluereturn->setText("0");
        setEditModeUI(checked);
        updateTotals();
        updateItemsCount();
        ui->purchbtncash->setChecked(true);
        emit payGroup->buttonClicked(ui->purchbtncash);
        loadData();
    });

    connect(payGroup, &QButtonGroup::buttonClicked, this, [=](QAbstractButton *btn){
        ui->purchbtncash->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/cash.svg"));
        ui->purchbtnvisa->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/credit-card3.svg"));

        if (btn == ui->purchbtncash)
            ui->purchbtncash->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/cash2.svg"));
        else if (btn == ui->purchbtnvisa)
            ui->purchbtnvisa->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/credit-card4.svg"));
    });

    ui->purchwidgetdate->setVisible(false);
    ui->purchwidgetnum->setVisible(false);

    refreshCompleter(isReturnMode);
    loadData();
}



void purchaseswindow::setEditModeUI(bool isEdit)
{
    isReturnMode = isEdit;

    if (isEdit) {
        // loadReturnData();
        refreshCompleter(isEdit);
        ui->purchmode->setText("وضع المرتجع");
        ui->purchbtnmode->setText("f8 إضافة مشتريات");
        ui->purchbtnmode->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/plus.svg"));
        ui->purchbtnmode->setShortcut(QKeySequence("F8"));
        ui->purchwidgetdate->setVisible(true);
        ui->purchwidgetnum->setVisible(true);
        ui->purchsearch->setPlaceholderText("ابحث عن فاتورة  برقم الفاتورة    (Ctrl+F)");
        ui->purchsearch->clear();
        ui->purchsearch->setFocus();
        ui->purchvaluedatereturn->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->purchvaluedate->setText("0");
        // ui->actionbtnvisa->setEnabled(false);
        ui->purchmode->setStyleSheet(
            "background: #FEF3C7;"
            "color: #92400E;"
            );
    } else {
        loadData();
        refreshCompleter(isEdit);
        ui->purchmode->setText("وضع الشراء");
        ui->purchbtnmode->setText("f8 إضافة مرتجع");
        ui->purchbtnmode->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/refresh2.svg"));
        ui->purchbtnmode->setShortcut(QKeySequence("F8"));
        ui->purchsearch->setPlaceholderText("ابحث عن دواء  بالاسم  أو  الباركود    (Ctrl+F)");
        ui->purchsearch->clear();
        ui->purchsearch->setFocus();
        ui->purchwidgetdate->setVisible(false);
        ui->purchwidgetnum->setVisible(false);
        ui->purchvaluedatereturn->setText("0");
        ui->purchvaluedate->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->actionbtnvisa->setEnabled(true);
        ui->purchmode->setStyleSheet(
            "background: #D1FAE5;"
            "color: #065F46;"
            );
    }
}

// ================= AUTOCOMPLETE =================
void purchaseswindow::refreshCompleter(bool mode)
{
    QStringList suggestions;

    if(!mode) // Sale mode
    {
        QList<Medicine> meds = Medicine::getAll();

        for(int i = 0; i < meds.size(); i++)
        {
            suggestions << meds[i].getName();
            suggestions << meds[i].getBarcode();
        }
    }
    // else // Return mode
    // {
    //     QList<Action> a = Action::getAll();

    //     for(int i = 0; i < a.size(); i++)
    //     {
    //         if(a[i].getInvoiceNumber().contains("INV")) // fix القوس
    //         {
    //             suggestions << a[i].getInvoiceNumber();
    //         }
    //     }
    // }

    // 🔥 إزالة التكرار + ترتيب
    suggestions.removeDuplicates();
    suggestions.sort(Qt::CaseInsensitive);

    // 🔥 امسح القديم لو موجود (مهم جدًا)
    if(ui->purchsearch->completer())
        delete ui->purchsearch->completer();

    // 🔥 إنشاء completer جديد
    QCompleter *completer = new QCompleter(suggestions, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    ui->purchsearch->setCompleter(completer);
}

void purchaseswindow::generateInvoiceNumber(bool mode)
{
    if(!isReturnMode && invoiceStarted) return;

    invoiceStarted = true;

    if(!mode) {
        QSqlQuery q(Database::instance());
        q.prepare("SELECT invoice_number FROM purchase_invoices "
                  "WHERE invoice_number LIKE 'PUR-%' "
                  "ORDER BY id DESC LIMIT 1");

        int count = 1;

        if(q.exec() && q.next())
        {
            QString last = q.value(0).toString();
            QString num = last.split("-").last();
            count = num.toInt() + 1;
        }
        invoiceNumber = QString("PUR-%1").arg(count, 5, 10, QChar('0'));
        ui->purchvalueinv->setText(invoiceNumber);

    } else {
        QSqlQuery q(Database::instance());
        q.prepare("SELECT invoice_number FROM purchase_invoices "
                  "WHERE invoice_number LIKE 'PRT-%' "
                  "ORDER BY id DESC LIMIT 1");

        int count = 1;

        if(q.exec() && q.next())
        {
            QString last = q.value(0).toString();
            QString num = last.split("-").last();
            count = num.toInt() + 1;
        }
        invoiceNumber = QString("PRT-%1").arg(count, 5, 10, QChar('0'));
        ui->purchvaluereturn->setText(invoiceNumber);
    }
}

QString purchaseswindow::getPaymentMethod()
{
    return ui->purchbtncash->isChecked() ? "كاش" : "تحويل";
}


void purchaseswindow::updateTotals()
{
    double total = 0;

    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        const CartItem &item = it.value();
        total += item.qty * item.purchasePrice;
    }

    ui->purchvaluetotal->setText(QString::number(total, 'f', 2));
}


void purchaseswindow::updateItemsCount()
{
    int totalItems = 0;

    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        totalItems += it.value().qty;
    }

    ui->purchvaluequantity->setText(QString::number(totalItems));
}

void purchaseswindow::loadData()
{
    model->removeRows(0, model->rowCount());

    model->setColumnCount(7);

    model->setHorizontalHeaderLabels({
        "اسم الدواء",
        "سعر الشراء",
        "الكمية",
        "تاريخ الانتهاء",
        "سعر البيع",
        "الإجمالي",
        "الاجراءات"
    });

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);

    QFont boldFont("Segoe UI", 10, QFont::Bold);

    int row = 0;

    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        int id = it.key();
        CartItem &item = it.value();

        Medicine m = Medicine::getById(id);

        QString name = m.getName();



        // ================= ITEMS =================
        auto *nameItem  = new QStandardItem(name);
        auto *totalItem = new QStandardItem(QString::number(item.purchasePrice * item.qty));
        auto *purchasePriceItem = new QStandardItem(QString::number(item.purchasePrice));
        auto *qtyItem = new QStandardItem(QString::number(item.qty));
        auto *emptyItem = new QStandardItem("");

        nameItem->setEditable(false);
        totalItem->setEditable(false);

        QList<QStandardItem*> items = {
                                        nameItem,
                                        purchasePriceItem,
                                        qtyItem,
                                        new QStandardItem(),
                                        new QStandardItem(),
                                        totalItem,
                                        emptyItem
        };

        model->appendRow(items);

        for (int i = 0; i < items.size(); i++)
        {
            items[i]->setTextAlignment(Qt::AlignCenter);
            items[i]->setForeground(gray);
            items[i]->setFont(boldFont);
        }

        nameItem->setForeground(dark);
        totalItem->setForeground(teal);

        nameItem->setFont(boldFont);
        totalItem->setFont(boldFont);

        // ================= QTY =================
        QLineEdit *qtyEdit = new QLineEdit(QString::number(item.qty));
        qtyEdit->setAlignment(Qt::AlignCenter);
        qtyEdit->setProperty("id", id);

        connect(qtyEdit, &QLineEdit::textChanged, this, [=]() {

            int id = qtyEdit->property("id").toInt();
            if (!cart.contains(id)) return;

            int q = qtyEdit->text().toInt();
            if (q < 1) q = 1;

            if (qtyEdit->text().toInt() != q)
                qtyEdit->setText(QString::number(q));

            cart[id].qty = q;

            updateTotals();
            updateItemsCount();

            double price = cart[id].purchasePrice;

            auto totalItem = model->item(row, 5);
            if (totalItem)
                totalItem->setText(QString::number(price * q));
        });

        qtyEdit->setStyleSheet(R"(
QLineEdit {
    background: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 6px;
    font-weight: bold;
    color: #065f46;
}
)");

        ui->purchcart->setIndexWidget(model->index(row, 2), qtyEdit);

        // ================= PURCHASE PRICE =================
        QLineEdit *purchaseEdit =
            new QLineEdit(QString::number(item.purchasePrice));

        purchaseEdit->setAlignment(Qt::AlignCenter);
        purchaseEdit->setProperty("id", id);

        connect(purchaseEdit, &QLineEdit::textChanged, this, [=]() {

            int id = purchaseEdit->property("id").toInt();
            if (!cart.contains(id)) return;

            double p = purchaseEdit->text().toDouble();
            if (p < 0) p = 0;

            if (purchaseEdit->text().toDouble() != p)
                purchaseEdit->setText(QString::number(p));

            cart[id].purchasePrice = p;

            updateTotals();

            int q = cart[id].qty;

            auto totalItem = model->item(row, 5);
            if (totalItem)
                totalItem->setText(QString::number(p * q));
        });

        purchaseEdit->setStyleSheet(R"(
QLineEdit {
    background: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 6px;
    font-weight: bold;
    color: #065f46;
}
)");

        ui->purchcart->setIndexWidget(model->index(row, 1), purchaseEdit);

        // ================= SALE PRICE =================
        QLineEdit *saleEdit =
            new QLineEdit(QString::number(item.salePrice));

        saleEdit->setAlignment(Qt::AlignCenter);
        saleEdit->setProperty("id", id);

        connect(saleEdit, &QLineEdit::textChanged, this, [=]() mutable {

            int id = saleEdit->property("id").toInt();

            bool ok;
            double s = saleEdit->text().toDouble(&ok);
            if (!ok || s < 0) s = 0;

            cart[id].salePrice = s;
        });

        saleEdit->setStyleSheet(R"(
QLineEdit {
    background: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 6px;
    font-weight: bold;
    color: #065f46;
}
)");

        ui->purchcart->setIndexWidget(model->index(row, 4), saleEdit);

        // ================= EXPIRY DATE =================
        QString dateText = item.expiryDate.isValid()
                               ? item.expiryDate.toString("yyyy-MM-dd")
                               : "اختر التاريخ";

        QPushButton *dateBtn = new QPushButton(dateText);

        dateBtn->setProperty("id", id);

        QCalendarWidget *cal = new QCalendarWidget();
        QDialog *popup = new QDialog(this);
        QVBoxLayout *l = new QVBoxLayout(popup);
        l->addWidget(cal);

        connect(dateBtn, &QPushButton::clicked, this, [=]() {
            popup->exec();
        });

        connect(cal, &QCalendarWidget::clicked, this, [=](QDate date) {

            int id = dateBtn->property("id").toInt();

            if (!cart.contains(id)) return;

            cart[id].expiryDate = date;
            dateBtn->setText(date.toString("yyyy-MM-dd"));

            popup->accept();
        });

        dateBtn->setCursor(Qt::PointingHandCursor);

        dateBtn->setStyleSheet(R"(
QPushButton {
    background-color: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 8px;
    color: #065f46;
    font-weight: bold;
}

QPushButton:hover {
    background-color: #D1FAE5;
}
)");

        ui->purchcart->setIndexWidget(model->index(row, 3), dateBtn);

        // ================= DELETE BUTTON =================
        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);

        QPushButton *deleteBtn = new QPushButton();
        deleteBtn->setObjectName("middelete");
        deleteBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/trash.svg"));
        deleteBtn->setIconSize(QSize(16,16));
        deleteBtn->setFixedSize(28,28);
        deleteBtn->setCursor(Qt::PointingHandCursor);

        layout->addWidget(deleteBtn);
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignCenter);

        ui->purchcart->setIndexWidget(model->index(row, 6), widget);

        connect(deleteBtn, &QPushButton::clicked, this, [this, id]() {

            auto reply = QMessageBox::question(this,
                                               "تأكيد",
                                               "هل تريد حذف الدواء؟",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                cart.remove(id);

                if (cart.isEmpty()) {
                    invoiceStarted = false;
                    invoiceNumber.clear();
                    ui->purchvalueinv->setText("0");
                }

                updateTotals();
                updateItemsCount();
                loadData();
            }
        });

        // ================= STYLING =================

        row++;
    }

    ui->purchcart->viewport()->update();

    auto header = ui->purchcart->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    // header->setSectionResizeMode(6, QHeaderView::Fixed);
    // ui->purchcart->setColumnWidth(6, 50);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->purchcart->setColumnWidth(3, 120);
}

purchaseswindow::~purchaseswindow()
{
    delete ui;
}