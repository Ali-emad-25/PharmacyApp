#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    homeCtrl = new homewindow(ui);
    purchasesCtrl = new purchaseswindow(ui);
    actionCtrl = new actionwindow(ui, homeCtrl);

    ui->lbllogo->setPixmap(QIcon("D:/Project_C++/PharmacyApp/icons/logo.png").pixmap(40));

    navGroup = new QButtonGroup(this);

    navGroup->addButton(ui->btnhome);
    navGroup->addButton(ui->btnmed);
    navGroup->addButton(ui->btnactions);
    navGroup->addButton(ui->btnpurch);
    navGroup->addButton(ui->btnrep);
    navGroup->addButton(ui->btnset);

    navGroup->setExclusive(true);

    connect(ui->btnhome, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->home);
        ui->homesearch->clear();
        ui->homesearch->setFocus();
        homeCtrl->currentFilter = Action::ALL;
        ui->hometablebtnall->setChecked(true);
        homeCtrl->loadData();
    });

    connect(ui->btnmed, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->medicines);
        // ui->pagesmid->setCurrentWidget(ui->pageveiwsmid);
        ui->btnmidadd->setChecked(false);
        ui->midsearch->clear();
        ui->midsearch->setFocus();
        currentFilter = Medicine::ALL;
        loadData();
    });

    connect(ui->btnactions, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->transactions);
        ui->actionsearch->clear();
        ui->actionsearch->setFocus();
    });

    connect(ui->btnpurch, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->purchases);
    });

    connect(ui->btnrep, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->reports);
    });

    connect(ui->btnset, &QPushButton::clicked, this, [=](){
        ui->stackedWidget->setCurrentWidget(ui->settings);
    });

    QShortcut *globalSearch = new QShortcut(QKeySequence("Ctrl+F"), this);

    connect(globalSearch, &QShortcut::activated, this, [=]() {

        QWidget *current = ui->stackedWidget->currentWidget();

        if (current == ui->home) {
            ui->homesearch->setFocus();
            ui->homesearch->selectAll();
        }
        else if (current == ui->medicines) {
            ui->midsearch->setFocus();
            ui->midsearch->selectAll();
        }
        else if (current == ui->transactions) {
            ui->actionsearch->setFocus();
            ui->actionsearch->selectAll();
        }

    });

    connect(ui->midsearch, &QLineEdit::textChanged, this, [=](const QString &text){
        searchText = text.trimmed();
        currentFilter = Medicine::ALL;
        loadData();
    });

    ui->btnhome->setChecked(true);
    ui->stackedWidget->setCurrentWidget(ui->home);

    connect(ui->btnexit, &QPushButton::released, this, [=](){

        ui->btnexit->setChecked(false);

        auto reply = QMessageBox::question(
            this,
            "خروج",
            "هل تريد إغلاق البرنامج؟",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            qApp->quit();
        }
    });

    // connect(ui->btnmidadd, &QPushButton::toggled, this, [=](bool checked) {

    //     if (checked) {
    //         ui->pagesmid->setCurrentWidget(ui->pageaddmid);
    //         ui->btnmidadd->setText("f8               رجوع");
    //         ui->btnmidadd->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/chevron-right.svg"));
    //         ui->btnmidadd->setShortcut(QKeySequence("F8"));
    //         ui->midsearch->clear();
    //         ui->midsearch->setEnabled(false);
    //         currentFilter = Medicine::ALL;
    //         loadData();

    //     } else {
    //         ui->pagesmid->setCurrentWidget(ui->pageveiwsmid);
    //         ui->btnmidadd->setText("f8      إضافة دواء");
    //         ui->btnmidadd->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/plus.svg"));
    //         ui->btnmidadd->setShortcut(QKeySequence("F8"));
    //         ui->midsearch->setEnabled(true);
    //         clearForm();
    //     }
    // });

    // connect(ui->availablevalue, &QPushButton::clicked, this, [=](){
    //     searchText.clear();
    //     currentFilter = Medicine::AVAILABLE;
    //     loadData();
    // });

    // connect(ui->lowvalue, &QPushButton::clicked, this, [=](){
    //     searchText.clear();
    //     currentFilter = Medicine::LOW;
    //     loadData();
    // });

    // connect(ui->expiredvalue, &QPushButton::clicked, this, [=](){
    //     searchText.clear();
    //     currentFilter = Medicine::EXPIRED;
    //     loadData();
    // });

    // connect(ui->totalvalue, &QPushButton::clicked, this, [=](){
    //     searchText.clear();
    //     currentFilter = Medicine::ALL;
    //     loadData();
    // });

    ui->btnhome->setCursor(Qt::PointingHandCursor);
    ui->btnmed->setCursor(Qt::PointingHandCursor);
    ui->btnactions->setCursor(Qt::PointingHandCursor);
    ui->btnpurch->setCursor(Qt::PointingHandCursor);
    ui->btnrep->setCursor(Qt::PointingHandCursor);
    ui->btnset->setCursor(Qt::PointingHandCursor);
    ui->btnexit->setCursor(Qt::PointingHandCursor);
    ui->btnmidadd->setCursor(Qt::PointingHandCursor);
    ui->midsave->setCursor(Qt::PointingHandCursor);
    ui->midcancel->setCursor(Qt::PointingHandCursor);
    // ui->totalvalue->setCursor(Qt::PointingHandCursor);
    // ui->expiredvalue->setCursor(Qt::PointingHandCursor);
    // ui->lowvalue->setCursor(Qt::PointingHandCursor);
    // ui->availablevalue->setCursor(Qt::PointingHandCursor);

    ui->sale_price->setValidator(new QDoubleValidator(0, 100000, 2, this));
    ui->min_quantity->setValidator(new QIntValidator(0, 100000, this));
    QRegularExpression rx("^\\d{13}$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->barcode->setValidator(validator);

    model = new QStandardItemModel(this);
    ui->midtable->setModel(model);

    ui->midtable->setAlternatingRowColors(true);
    ui->midtable->setShowGrid(false);
    ui->midtable->setFrameShape(QFrame::NoFrame);
    ui->midtable->verticalHeader()->setVisible(false);

    ui->midtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->midtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->midtable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->midtable->setFocusPolicy(Qt::NoFocus);

    ui->midtable->setTextElideMode(Qt::ElideNone);

    ui->midtable->verticalHeader()->setDefaultSectionSize(45);
    ui->midtable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->midtable->horizontalHeader()->setStretchLastSection(true);
    ui->midtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->midtable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

    // ===== Scroll =====
    ui->midtable->horizontalScrollBar()->setCursor(Qt::PointingHandCursor);
    ui->midtable->verticalScrollBar()->setCursor(Qt::PointingHandCursor);

    ui->hometable->setStyleSheet("border: none;");
    ui->actiontable->setStyleSheet("border: none;");
    ui->purchcart->setStyleSheet("border: none;");
    ui->purchtable->setStyleSheet("border: none;");

    loadData();
    // updateStats();
}


// bool MainWindow::isValidEAN13(const QString &code)
// {
//     if (code.length() != 13) return false;

//     int sum = 0;

//     for (int i = 0; i < 12; i++)
//     {
//         int digit = code[i].digitValue();

//         if (i % 2 == 0)
//             sum += digit;
//         else
//             sum += digit * 3;
//     }

//     int checkDigit = (10 - (sum % 10)) % 10;

//     return checkDigit == code[12].digitValue();
// }

void MainWindow::on_midsave_clicked()
{
    QString name = ui->name->text();
    QString category = ui->category->text();
    double sale_price = ui->sale_price->text().toDouble();
    // int quantity = ui->quantity->text().toInt();
    int min_quantity = ui->min_quantity->text().toInt();
    // QString supplier = ui->supplier->text();
    QString barcode = ui->barcode->text();
    // QString expiry = ui->expiry_date->date().toString(Qt::ISODate);

    if(ui->name->text().isEmpty() ||
        ui->barcode->text().isEmpty() ||
        ui->category->text().isEmpty() ||
        // ui->supplier->text().isEmpty() ||
        ui->sale_price->text().isEmpty() ||
        // ui->quantity->text().isEmpty() ||
        ui->min_quantity->text().isEmpty())
    {
        QMessageBox::warning(this,"تنبيه","ادخل جميع البيانات");
        return;
    }

    // if(ui->expiry_date->date() <= QDate::currentDate())
    // {
    //     QMessageBox::warning(this,"تنبيه","لا يمكن ادخال تاريخ اليوم او تاريخ سابق");
    //     return;
    // }

    // if(barcode.length() != 13 || !isValidEAN13(barcode))
    // {
    //     QMessageBox::warning(this,"تنبيه","الباركود غير صحيح (EAN-13 غير صالح)");
    //     return;
    // }

    Medicine m(currentId,name,category,0,sale_price,min_quantity,barcode);

    bool success;

    if(currentId == 0){
        success = m.add();
    }else{
        success = m.update();
    }

    if(success){
        QMessageBox::information(this,"نجاح","تم الحفظ بنجاح");
        clearForm();
        loadData();
        // updateStats();
        setEditModeUI(false);
        currentId = 0;
        actionCtrl->refreshCompleter(actionCtrl->isReturnMode);
        if(actionCtrl->isReturnMode) {
            actionCtrl->loadReturnData();
        } else {
            actionCtrl->loadData();
        }
        purchasesCtrl->refreshCompleter(purchasesCtrl->isReturnMode);;
        // ui->pagesmid->setCurrentWidget(ui->pageveiwsmid);
        // ui->btnmidadd->setChecked(false);

    } else {
        QMessageBox::critical(this,"خطأ","فشل الحفظ (ممكن الباركود مكرر)");
    }
}


void MainWindow::on_midcancel_clicked()
{
    clearForm();
}
void MainWindow::clearForm()
{
    ui->name->clear();
    ui->category->clear();
    ui->sale_price->clear();
    // ui->quantity->clear();
    ui->min_quantity->clear();
    // ui->supplier->clear();
    ui->barcode->clear();
    // ui->expiry_date->setDate(QDate::currentDate());
    ui->name->setFocus();
    setEditModeUI(false);

    currentId = 0;
}


void MainWindow::setEditModeUI(bool isEdit)
{
    if (isEdit) {
        ui->midmode->setText("وضع التعديل");
        ui->midmode->setStyleSheet(
            "background: #FEF3C7;"
            "color: #92400E;"
            );
    } else {
        ui->midmode->setText("وضع الإضافة");
        ui->midmode->setStyleSheet(
            "background-color:#D1FAE5;"
            "color:#065F46;"
            );
    }
}


// void MainWindow::updateStats()
// {
//     QList<Medicine> list = Medicine::getAll();

//     int total = 0;
//     int available = 0;
//     int low = 0;
//     int expired = 0;

//     for (int i = 0; i < list.size(); i++)
//     {
//         QString status = list[i].getStatus();

//         total++;

//         if (status == "متوفر")
//             available++;
//         else if (status == "ناقص")
//             low++;
//         else if (status == "منتهي الصلاحية")
//             expired++;
//     }

//     // ui->totalvalue->setText(QString::number(total));
//     // ui->availablevalue->setText(QString::number(available));
//     // ui->lowvalue->setText(QString::number(low));
//     // ui->expiredvalue->setText(QString::number(expired));
// }


void MainWindow::loadData()
{
    model->clear();

    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({
        "الكود", "اسم الدواء", "الفئة", "السعر", "الكمية", "الإجراءات"
    });

    QList<Medicine> list;

    if (!searchText.isEmpty())
    {
        list = Medicine::search(searchText);
    }
    else
    {
        list = Medicine::getAll();
    }

    medicinesMap.clear();

    for (int i = 0; i < list.size(); i++)
    {
        const Medicine &m = list[i];
        medicinesMap.insert(m.getId(), m);
    }

    QColor gray(107,114,128);
    QColor dark(17,24,39);
    QColor teal(20,184,166);

    QColor green(34,197,94);
    QColor yellow(234,179,8);
    QColor purple(168,85,247);
    QColor red(239,68,68);

    QFont normalFont("Segoe UI", 10);
    QFont boldFont("Segoe UI", 10, QFont::Bold);

    for (int row = 0; row < list.size(); row++)
    {
        const Medicine &m = list[row];
        int id = m.getId();

        auto *barcodeItem = new QStandardItem(m.getBarcode());
        auto *nameItem    = new QStandardItem(m.getName());
        auto *catItem     = new QStandardItem(m.getCategory());
        auto *salepriceItem   = new QStandardItem(QString::number(m.getSalePrice()));

        QList<QStandardItem*> items = {
            barcodeItem, nameItem, catItem, salepriceItem,new QStandardItem("1"), new QStandardItem("")
        };

        model->appendRow(items);

        for (int i = 0; i < items.size(); i++)
        {
            QStandardItem *item = items[i];
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(gray);
            item->setFont(normalFont);
        }

        nameItem->setFont(boldFont);
        salepriceItem->setForeground(teal);
        salepriceItem->setFont(boldFont);

        QColor color;

        // if (m.getStatus() == "متوفر") color = green;
        // else if (m.getStatus() == "ناقص") color = yellow;
        // else if (m.getStatus() == "غير متوفر") color = purple;
        // else color = red;

        // statusItem->setForeground(color);
        // qtyItem->setForeground(color);
        // qtyItem->setFont(boldFont);

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);

        QPushButton *editBtn = new QPushButton();
        QPushButton *deleteBtn = new QPushButton();

        editBtn->setObjectName("midedit");
        deleteBtn->setObjectName("middelete");

        editBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/edit.svg"));
        deleteBtn->setIcon(QIcon("D:/Project_C++/PharmacyApp/icons/trash.svg"));

        editBtn->setIconSize(QSize(16,16));
        deleteBtn->setIconSize(QSize(16,16));

        editBtn->setFixedSize(28,28);
        deleteBtn->setFixedSize(28,28);

        editBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setCursor(Qt::PointingHandCursor);

        layout->addWidget(editBtn);
        layout->addWidget(deleteBtn);
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignCenter);

        ui->midtable->setIndexWidget(model->index(row, 5), actionWidget);

        connect(editBtn, &QPushButton::clicked, this, [this, id]() {

            Medicine m = medicinesMap.value(id);

            ui->name->setText(m.getName());
            ui->category->setText(m.getCategory());
            ui->sale_price->setText(QString::number(m.getSalePrice()));
            // ui->quantity->setText(QString::number(m.getQuantity()));
            ui->min_quantity->setText(QString::number(m.getMinQuantity()));
            ui->barcode->setText(m.getBarcode());

            currentId = id;
            setEditModeUI(true);
        });

        connect(deleteBtn, &QPushButton::clicked, this, [this, id]() {

            auto reply = QMessageBox::question(this,
                                               "تأكيد",
                                               "هل تريد حذف الدواء؟",
                                               QMessageBox::Yes | QMessageBox::No
                                               );

            if (reply == QMessageBox::Yes)
            {
                QSqlQuery query(Database::instance());
                query.prepare("DELETE FROM medicines WHERE id=:id");
                query.bindValue(":id", id);

                if (!query.exec()) {
                    QMessageBox::critical(this, "خطأ", query.lastError().text());
                } else {
                    loadData();
                    actionCtrl->refreshCompleter(actionCtrl->isReturnMode);
                    if(actionCtrl->isReturnMode) {
                        actionCtrl->loadReturnData();
                    } else {
                        actionCtrl->loadData();
                    }
                    // updateStats();
                }
            }
        });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}