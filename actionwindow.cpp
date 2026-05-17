#include "actionwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "medicine.h"
#include "action.h"

actionwindow::actionwindow(Ui::MainWindow *ui, homewindow *home)
{
    this->ui = ui;
    this->homeCtrl = home;

    ui->actiondatesale->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    ui->actionvaluesale->setText("0");

    ui->btnmode->setCursor(Qt::PointingHandCursor);
    ui->actionbtncash->setCursor(Qt::PointingHandCursor);
    ui->actionbtnvisa->setCursor(Qt::PointingHandCursor);
    ui->actionbtndone->setCursor(Qt::PointingHandCursor);
    ui->actionbtncancel->setCursor(Qt::PointingHandCursor);

    // ================= PAYMENT =================
    payGroup = new QButtonGroup(this);
    payGroup->addButton(ui->actionbtncash);
    payGroup->addButton(ui->actionbtnvisa);
    payGroup->setExclusive(true);

    ui->actionbtncash->setChecked(true);

    // ================= TABLE =================
    model = new QStandardItemModel(this);
    ui->actiontable->setModel(model);

    ui->actiontable->verticalHeader()->setVisible(false);
    ui->actiontable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->actiontable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->actiontable->setTextElideMode(Qt::ElideNone);
    ui->actiontable->verticalHeader()->setDefaultSectionSize(45);
    ui->actiontable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->actiontable->setShowGrid(false);
    ui->actiontable->setFocusPolicy(Qt::NoFocus);
    ui->actiontable->setFrameShape(QFrame::NoFrame);
    ui->actiontable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->actiontable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);



    // ================= ENTER = ADD TO CART =================
    connect(ui->actionsearch, &QLineEdit::returnPressed,
            this, [this, ui]() {

                QString key = ui->actionsearch->text().trimmed();
                if (key.isEmpty()) {
                    if (!isReturnMode) {
                        QMessageBox::warning(this, "خطأ", "من فضلك أدخل اسم الدواء او الباركود");
                        return;

                    } else {
                        QMessageBox::warning(this, "خطأ", "من فضلك أدخل رقم الفاتورة");
                        return;
                    }
                }

                if(!isReturnMode) {
                    QList<Medicine> result = Medicine::search(key);
                    if(result.isEmpty()) {
                        QMessageBox::warning(this, "خطأ", "لا يوجد دواء مطابق لهذا البحث");
                        return;
                    }

                    Medicine m = result.first();

                    // ================= CHECK STATUS =================
                    // QString status = m.getStatus();

                    // if(status == "غير متوفر")
                    // {
                    //     QMessageBox::warning(this, "خطأ", "هذا الدواء غير متوفر في المخزون");
                    //     ui->actionsearch->clear();
                    //     return;
                    // }

                    // if(status == "منتهي الصلاحية")
                    // {
                    //     QMessageBox::warning(this, "خطأ", "هذا الدواء منتهي الصلاحية");
                    //     ui->actionsearch->clear();
                    //     return;
                    // }

                    // ================= ADD TO CART =================
                    int id = m.getId();

                    if(cart.isEmpty())
                        generateInvoiceNumber(false);

                    if(cart.contains(id))
                        cart[id]++;
                    else
                        cart.insert(id, 1);

                    loadData();
                    updateTotals();
                    updateItemsCount();

                } else {
                    QList<Action> result = Action::search(key);

                    if(result.isEmpty())
                    {
                        QMessageBox::warning(this, "خطأ", "لا يوجد فاتورة مطابقة لهذا البحث");
                        return;
                    }

                    Action a = result.first();

                    if(a.getType() == "مرتجع")
                    {
                        QMessageBox::warning(this, "خطأ", "هذه الفاتورة مرتجعة بالفعل");
                        return;
                    }

                    if(cart.isEmpty()) {
                        currentInvoiceId = a.getId();

                        cart = Action::getInvoiceItems(a.getId());

                        invoiceStarted = true;
                        generateInvoiceNumber(true);

                        ui->actiondatesale->setText(a.getDate());
                        ui->actionvaluesale->setText(a.getInvoiceNumber());

                        loadReturnData();
                        updateTotals();
                        updateItemsCount();

                        QMessageBox::information(this, "تم", "تم تحميل الفاتورة بنجاح");
                    } else {
                        QMessageBox::warning(this, "خطأ", "لا يمكن تحميل الفاتورة الان");
                    }
                }

                ui->actionsearch->clear();
            });

    connect(ui->actionbtndone, &QPushButton::clicked, this, [this, ui]() {

        if (cart.isEmpty())
        {
            if (!isReturnMode) {
                QMessageBox::critical(this, "خطأ", "قم بإضافة دواء أولاً");
                return;
            } else {
                QMessageBox::critical(this, "خطأ", "قم بإضافة فاتورة أولاً");
                return;
            }
        }

        // ================= COMMON DATA =================
        QString date = QDate::currentDate().toString("yyyy-MM-dd");

        // ================= SELL MODE =================
        if (!isReturnMode)
        {
            QString invoiceNo = invoiceNumber;

            double subtotal = ui->actionvaluesubtotal->text().toDouble();
            double tax      = ui->actionvaluetax->text().toDouble();
            double total    = ui->actionvaluetotal->text().toDouble();

            int id = Action::saveInvoice(
                invoiceNo,
                date,
                subtotal,
                tax,
                total,
                getPaymentMethod(),
                cart
                );

            if (id == -1)
            {
                QMessageBox::critical(this, "خطأ", "فشل في حفظ فاتورة البيع");
                return;
            }

            QMessageBox::information(this, "نجاح", "تم حفظ فاتورة البيع");

            cart.clear();
            returnReasons.clear();
            invoiceStarted = false;
            invoiceNumber.clear();
            loadData();
        }

        // ================= RETURN MODE =================
        else
        {
            QString returnInvoiceNo = invoiceNumber;

            double subtotal = ui->actionvaluesubtotal->text().toDouble();
            double tax      = ui->actionvaluetax->text().toDouble();
            double total    = ui->actionvaluetotal->text().toDouble();

            QMap<int, QPair<int, QString>> returnCart;

            for (auto it = cart.begin(); it != cart.end(); ++it)
            {
                int id = it.key();
                int qty = it.value();

                int originalQty = Action::getInvoiceItems(currentInvoiceId).value(id);
                int alreadyReturned = Action::getAlreadyReturnedQty(currentInvoiceId, id);

                int availableQty = originalQty - alreadyReturned;

                int finalQty = qMin(qty, availableQty);

                if (finalQty <= 0)
                    continue;

                QString reason = returnReasons.value(id, "other");

                returnCart[id] = qMakePair(finalQty, reason);
            }

            if (returnCart.isEmpty())
            {
                QMessageBox::warning(this,
                                     "خطأ",
                                     "لا توجد عناصر صالحة للمرتجع");

                return;
            }

            int id = Action::saveReturnInvoice(
                returnInvoiceNo,
                currentInvoiceId,
                date,
                subtotal,
                tax,
                total,
                returnCart
                );

            if (id == -1)
            {
                QMessageBox::critical(this, "خطأ", "فشل في حفظ المرتجع");
                return;
            }

            QMessageBox::information(this, "نجاح", "تم حفظ المرتجع");

            cart.clear();
            returnReasons.clear();
            invoiceStarted = false;
            invoiceNumber.clear();
            loadReturnData();
            ui->actiondatesale->setText("0");
        }

        // ================= RESET SYSTEM =================
        ui->actionvaluesale->setText("0");
        ui->actionvaluereturn->setText("0");

        updateTotals();
        updateItemsCount();

        homeCtrl->searchText.clear();
        homeCtrl->currentFilter = Action::ALL;
        homeCtrl->loadData();
    });

    connect(ui->btnmode, &QPushButton::toggled, this, [=](bool checked) {
        cart.clear();
        returnReasons.clear();
        invoiceStarted = false;
        invoiceNumber.clear();
        ui->actionvaluesale->setText("0");
        ui->actionvaluereturn->setText("0");
        setEditModeUI(checked);
        updateTotals();
        updateItemsCount();
        ui->actionbtncash->setChecked(true);
        emit payGroup->buttonClicked(ui->actionbtncash);
    });

    connect(payGroup, &QButtonGroup::buttonClicked, this, [=](QAbstractButton *btn){
        ui->actionbtncash->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/cash.svg"));
        if(!isReturnMode) {
            ui->actionbtnvisa->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/credit-card3.svg"));
        } else {
            ui->actionbtnvisa->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/credit-card4.svg"));
        }

        if (btn == ui->actionbtncash)
            ui->actionbtncash->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/cash2.svg"));
        else if (btn == ui->actionbtnvisa)
            ui->actionbtnvisa->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/credit-card4.svg"));
    });

    ui->actionwidgetreturnnum->setVisible(false);
    ui->actionwidgetreturndate->setVisible(false);

    refreshCompleter(isReturnMode);
    loadData();
}



void actionwindow::setEditModeUI(bool isEdit)
{
    isReturnMode = isEdit;

    if (isEdit) {
        loadReturnData();
        refreshCompleter(isEdit);
        ui->actionmode->setText("وضع المرتجع");
        ui->btnmode->setText("f8           بيع جديد");
        ui->btnmode->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/plus.svg"));
        ui->btnmode->setShortcut(QKeySequence("F8"));
        ui->actionwidgetreturnnum->setVisible(true);
        ui->actionwidgetreturndate->setVisible(true);
        ui->actionsearch->setPlaceholderText("ابحث عن فاتورة  برقم الفاتورة    (Ctrl+F)");
        ui->actionsearch->clear();
        ui->actionsearch->setFocus();
        ui->actiondatereturn->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->actiondatesale->setText("0");
        ui->actionbtnvisa->setEnabled(false);
        ui->actionmode->setStyleSheet(
            "background: #FEF3C7;"
            "color: #92400E;"
            );
        ui->actionbtnvisa->setStyleSheet(
            "background: #FFFFFF;"
            "color: #CBD5E1;"
            "border: 2px solid #F1F5F9;"
            "font-weight: bold;"
            "font-size: 13px;"
            "padding: 13px 10px;"
            "border-radius: 10px;"
            );
    } else {
        loadData();
        refreshCompleter(isEdit);
        ui->actionmode->setText("وضع البيع");
        ui->btnmode->setText("f8             مرتجع");
        ui->btnmode->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/refresh2.svg"));
        ui->btnmode->setShortcut(QKeySequence("F8"));
        ui->actionsearch->setPlaceholderText("ابحث عن دواء  بالاسم  أو  الباركود    (Ctrl+F)");
        ui->actionsearch->clear();
        ui->actionsearch->setFocus();
        ui->actionwidgetreturnnum->setVisible(false);
        ui->actionwidgetreturndate->setVisible(false);
        ui->actiondatereturn->setText("0");
        ui->actiondatesale->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->actionbtnvisa->setEnabled(true);
        ui->actionmode->setStyleSheet(
            "background: #D1FAE5;"
            "color: #065F46;"
            );
        ui->actionbtnvisa->setStyleSheet("");
    }
}

// ================= AUTOCOMPLETE =================
void actionwindow::refreshCompleter(bool mode)
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
    else // Return mode
    {
        QList<Action> a = Action::getAll();

        for(int i = 0; i < a.size(); i++)
        {
            if(a[i].getInvoiceNumber().contains("INV")) // fix القوس
            {
                suggestions << a[i].getInvoiceNumber();
            }
        }
    }

    // 🔥 إزالة التكرار + ترتيب
    suggestions.removeDuplicates();
    suggestions.sort(Qt::CaseInsensitive);

    // 🔥 امسح القديم لو موجود (مهم جدًا)
    if(ui->actionsearch->completer())
        delete ui->actionsearch->completer();

    // 🔥 إنشاء completer جديد
    QCompleter *completer = new QCompleter(suggestions, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    ui->actionsearch->setCompleter(completer);
}

void actionwindow::generateInvoiceNumber(bool mode)
{
    if(!isReturnMode && invoiceStarted) return;

    invoiceStarted = true;

    if(!mode) {
        QSqlQuery q(Database::instance());
        q.prepare("SELECT invoice_number FROM invoices "
                  "WHERE invoice_number LIKE 'INV-%' "
                  "ORDER BY id DESC LIMIT 1");

        int count = 1;

        if(q.exec() && q.next())
        {
            QString last = q.value(0).toString();
            QString num = last.split("-").last();
            count = num.toInt() + 1;
        }
        invoiceNumber = QString("INV-%1").arg(count, 5, 10, QChar('0'));
        ui->actionvaluesale->setText(invoiceNumber);

    } else {
        QSqlQuery q(Database::instance());
        q.prepare("SELECT invoice_number FROM invoices "
                  "WHERE invoice_number LIKE 'RET-%' "
                  "ORDER BY id DESC LIMIT 1");

        int count = 1;

        if(q.exec() && q.next())
        {
            QString last = q.value(0).toString();
            QString num = last.split("-").last();
            count = num.toInt() + 1;
        }
        invoiceNumber = QString("RET-%1").arg(count, 5, 10, QChar('0'));
        ui->actionvaluereturn->setText(invoiceNumber);
    }
}

QString actionwindow::getPaymentMethod()
{
    return ui->actionbtncash->isChecked() ? "كاش" : "فيزا";
}

void actionwindow::updateTotals()
{
    double subtotal = 0;

    for(auto it = cart.begin(); it != cart.end(); ++it)
    {
        int id = it.key();
        int qty = it.value();

        Medicine m = Medicine::getById(id);

        subtotal += m.getSalePrice() * qty;
    }

    double tax = subtotal *  0.14;
    double total = subtotal + tax;

    ui->actionvaluesubtotal->setText(QString::number(subtotal, 'f', 2));
    ui->actionvaluetax->setText(QString::number(tax, 'f', 2));

    if(!isReturnMode) {
        ui->actionvaluetotal->setStyleSheet("color: #14b8a6;");
        ui->actionpoundtotal->setStyleSheet("color: #14b8a6;");
    } else {
        ui->actionvaluetotal->setStyleSheet("color: #ef5555;");
        ui->actionpoundtotal->setStyleSheet("color: #ef5555;");
    }

    ui->actionvaluetotal->setText(QString::number(total, 'f', 2));

}

void actionwindow::updateItemsCount()
{
    int totalItems = 0;

    for(auto it = cart.begin(); it != cart.end(); ++it)
    {
        totalItems += it.value();
    }

    ui->actionvalequantity->setText(QString::number(totalItems));
}


void actionwindow::loadData()
{
    model->removeRows(0, model->rowCount());

    model->setColumnCount(5);
    model->setHorizontalHeaderLabels({
        "اسم الدواء", "السعر", "الكمية", "الإجمالي", "الإجراءات"
    });

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);

    QFont normalFont("Segoe UI", 10);
    QFont boldFont("Segoe UI", 10, QFont::Bold);

    int row = 0;

    for (auto it = cart.begin(); it != cart.end(); ++it)
    {
        int id = it.key();
        int qty = it.value();

        Medicine m = Medicine::getById(id);

        QString name = m.getName();

        if(name.length() > 10)
        {
            name = name.left(10) + "\n" + name.mid(10);
        }

        auto *nameItem  = new QStandardItem(name);
        auto *priceItem = new QStandardItem(QString::number(m.getSalePrice()));
        auto *totalItem = new QStandardItem(QString::number(m.getSalePrice() * qty));
        auto *emptyItem = new QStandardItem("");

        nameItem->setEditable(false);
        priceItem->setEditable(false);
        totalItem->setEditable(false);

        QList<QStandardItem*> items = {
            nameItem, priceItem, new QStandardItem(), totalItem, emptyItem
        };

        model->appendRow(items);

        // ===== QLineEdit =====
        QLineEdit *qtyEdit = new QLineEdit(QString::number(qty));
        qtyEdit->setAlignment(Qt::AlignCenter);

        qtyEdit->setProperty("id", id);

        qtyEdit->setStyleSheet(R"(
QLineEdit {
    background: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 6px;
    font-weight: bold;
    color: #065f46;
}
)");

        ui->actiontable->setIndexWidget(model->index(row, 2), qtyEdit);

        // ===== SIGNAL =====
        connect(qtyEdit, &QLineEdit::returnPressed,
                this, [=]() {

                    int id = qtyEdit->property("id").toInt();

                    if (!cart.contains(id)) return;

                    Medicine m = Medicine::getById(id);
                    // int maxQty = m.getQuantity();

                    int qty = qtyEdit->text().toInt();

                    // ================= MIN CHECK =================
                    if (qty < 1)
                    {
                        qty = 1;
                        qtyEdit->setText("1");
                    }

                    // ================= MAX CHECK =================
                    // if (!isReturnMode && qty > maxQty)
                    // {
                    //     QMessageBox::warning(this,
                    //                          "الكمية غير متاحة",
                    //                          "الحد الأقصى المتاح هو: " + QString::number(maxQty));

                    //     qty = maxQty;
                    //     qtyEdit->setText(QString::number(maxQty));
                    // }

                    // ================= UPDATE CART =================
                    cart[id] = qty;

                    updateTotals();
                    updateItemsCount();

                    // ================= UPDATE TOTAL =================
                    auto priceItem = model->item(row, 1);
                    auto totalItem = model->item(row, 3);

                    if (priceItem && totalItem)
                    {
                        double price = priceItem->text().toDouble();
                        totalItem->setText(QString::number(price * qty));
                    }
                });

        for (int i = 0; i < items.size(); i++)
        {
            items[i]->setTextAlignment(Qt::AlignCenter);
            items[i]->setForeground(gray);
            items[i]->setFont(boldFont);
        }

        nameItem->setForeground(dark);
        totalItem->setForeground(teal);

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

        ui->actiontable->setIndexWidget(model->index(row, 4), widget);

        connect(deleteBtn, &QPushButton::clicked, this, [this, id]() {

            auto reply = QMessageBox::question(this,
                                               "تأكيد",
                                               "هل تريد حذف الدواء؟",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                cart.remove(id);
                loadData();
                updateTotals();
                updateItemsCount();

                if(cart.isEmpty())
                {
                    invoiceStarted = false;
                    invoiceNumber.clear();
                    ui->actionvaluesale->setText("0");
                }
            }
        });

        row++;
    }

    ui->actiontable->viewport()->update();
}

void actionwindow::loadReturnData()
{
    model->removeRows(0, model->rowCount());

    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({
        "اسم الدواء",
        "السعر",
        "الكمية الأصلية",
        "الكمية المرتجعة",
        "الإجمالي",
        "السبب"
    });

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);
    QColor red(239,68,68);

    QFont boldFont("Segoe UI", 10, QFont::Bold);

    int row = 0;

    QMap<int, int> temp = cart;

    for (auto it = temp.begin(); it != temp.end(); ++it)
    {
        int id = it.key();
        // int originalQty = it.value();

        cart[id] = 0;

        Medicine m = Medicine::getById(id);
        int originalQty = Action::getInvoiceItems(currentInvoiceId).value(id);

        QString name = m.getName();

        // if (name.length() > 9)
        // {
        //     int breakIndex = name.lastIndexOf(' ', 9);

        //     // لو مفيش مسافة قبل 12
        //     if (breakIndex == -1)
        //         breakIndex = name.indexOf(' ');

        //     // لو لسه مفيش (كلمة واحدة طويلة)
        //     if (breakIndex == -1)
        //         breakIndex = 9;

        //     name.insert(breakIndex, "\n");
        // }

        auto *nameItem = new QStandardItem(name);
        auto *priceItem = new QStandardItem(QString::number(m.getSalePrice()));
        auto *origItem  = new QStandardItem(QString::number(originalQty));
        auto *totalItem = new QStandardItem(QString::number(0));

        nameItem->setEditable(false);
        priceItem->setEditable(false);
        origItem->setEditable(false);
        totalItem->setEditable(false);

        QList<QStandardItem*> items = {
            nameItem,
            priceItem,
            origItem,
            new QStandardItem(),
            totalItem,
            new QStandardItem()
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

        QLineEdit *qtyEdit = new QLineEdit("0");
        qtyEdit->setAlignment(Qt::AlignCenter);

        qtyEdit->setProperty("id", id);

        qtyEdit->setStyleSheet(R"(
QLineEdit {
    background: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 6px;
    font-weight: bold;
    color: #065f46;
}
)");

        ui->actiontable->setIndexWidget(model->index(row, 3), qtyEdit);

        // ===== SIGNAL =====
        connect(qtyEdit, &QLineEdit::returnPressed, this, [=]()
                {
                    int id = qtyEdit->property("id").toInt();

                    int returnQty = qtyEdit->text().toInt();

                    // ================= GET AVAILABLE =================
                    int alreadyReturned = Action::getAlreadyReturnedQty(currentInvoiceId, id);
                    int availableQty = originalQty - alreadyReturned;

                    // ================= VALIDATION =================
                    if (returnQty < 1)
                        returnQty = 1;

                    if (returnQty > availableQty)
                    {
                        QMessageBox::warning(this,
                                             "تحذير",
                                             "المتاح للمرتجع فقط: " + QString::number(availableQty));

                        returnQty = availableQty;
                    }

                    qtyEdit->setText(QString::number(returnQty));

                    // ================= UPDATE CART =================
                    cart[id] = returnQty;

                    // ================= CALC TOTAL =================
                    double price = priceItem->text().toDouble();
                    double total = price * returnQty;

                    model->item(row, 4)->setText(QString::number(total));

                    updateTotals();
                    updateItemsCount();
                });

        QComboBox *combo = new QComboBox();

        combo->addItem("دواء خطأ", "wrong_medicine");
        combo->addItem("تغيير رأي العميل", "customer_changed_mind");
        combo->addItem("تلف في العبوة", "damaged_package");
        combo->addItem("أخرى", "other");

        combo->setProperty("id", id);
        combo->setCursor(Qt::PointingHandCursor);

        combo->setEditable(true);
        combo->lineEdit()->setReadOnly(true);
        combo->lineEdit()->setPlaceholderText("اختر السبب");
        combo->setCurrentIndex(-1);

        combo->setStyleSheet(R"(
QComboBox {
    background-color: #E6FBF1;
    border: 2px solid #E6FBF1;
    border-radius: 8px;
    font-weight: bold;
    color: #065f46;
}

QComboBox::drop-down {
    border: none;
    width: 28px;
}

QComboBox::down-arrow {
    image: url(D:/Project_C++/PharmacyApp/icons/caret-down.svg);
    width: 14px;
    height: 14px;
}
)");
        ui->actiontable->setIndexWidget(model->index(row, 5), combo);

        connect(combo, &QComboBox::currentIndexChanged, this, [=](int index)
                {
                    if (index == 0) return;

                    int id = combo->property("id").toInt();
                    QString reason = combo->currentData().toString();

                    returnReasons[id] = reason;

                    model->item(combo->property("row").toInt(), 5)
                        ->setText(combo->currentText());
                });
        combo->view()->setFixedWidth(130);
        combo->setFixedWidth(100);

        row++;
    }

    ui->actiontable->viewport()->update();
}

actionwindow::~actionwindow()
{
    delete ui;
}