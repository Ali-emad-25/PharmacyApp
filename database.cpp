#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QSqlDatabase Database::instance()
{
    if (QSqlDatabase::contains("pharmacy_connection"))
        return QSqlDatabase::database("pharmacy_connection");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "pharmacy_connection");
    db.setDatabaseName("pharmacy.db");

    if (!db.open()) {
        qDebug() << "DB Open Error:" << db.lastError().text();
    }

    return db;
}

bool Database::init()
{
    QSqlDatabase db = instance();

    if (!db.isOpen()) {
        qDebug() << "DB Init Error:" << db.lastError().text();
        return false;
    }

    QSqlQuery q(db);
    if (!q.exec("PRAGMA foreign_keys = ON;")) {
        qDebug() << "Foreign Keys Error:" << q.lastError().text();
    }

    createTables();
    return true;
}

void Database::createTables()
{
    QSqlQuery q(instance());

    // ================= MEDICINES =================
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS medicines (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            category TEXT,
            quantity INTEGER NOT NULL,
            sale_price REAL NOT NULL,
            min_quantity INTEGER,
            barcode TEXT UNIQUE
        )
    )")) {
        qDebug() << "Create medicines Error:" << q.lastError().text();
    }

    // ================= INVOICES (SALES) =================
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS invoices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            invoice_number TEXT UNIQUE NOT NULL,

            type TEXT CHECK(type IN ('بيع', 'مرتجع')) NOT NULL,
            date TEXT NOT NULL,

            subtotal REAL NOT NULL DEFAULT 0,
            tax REAL NOT NULL DEFAULT 0,
            total REAL NOT NULL DEFAULT 0,

            payment_method TEXT CHECK(payment_method IN ('كاش', 'فيزا')) NOT NULL,

            reference_invoice_id INTEGER,

            FOREIGN KEY(reference_invoice_id)
                REFERENCES invoices(id)
                ON DELETE SET NULL
                ON UPDATE CASCADE
        )
    )")) {
        qDebug() << "Create invoices Error:" << q.lastError().text();
    }

    // ================= PURCHASE INVOICES =================
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS purchase_invoices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            invoice_number TEXT UNIQUE NOT NULL,
            type TEXT CHECK(type IN ('شراء', 'مرتجع')) NOT NULL,
            date TEXT NOT NULL,
            supplier TEXT NOT NULL,

            total REAL NOT NULL DEFAULT 0,
            payment_method TEXT CHECK(payment_method IN ('كاش', 'تحويل')) NOT NULL,

            reference_invoice_id INTEGER,

            FOREIGN KEY(reference_invoice_id)
                REFERENCES purchase_invoices(id)
                ON DELETE SET NULL
                ON UPDATE CASCADE
        )
    )")) {
        qDebug() << "Create purchase_invoices Error:" << q.lastError().text();
    }

    // ================= PURCHASE ITEMS (STOCK / BATCHES) =================
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS purchase_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            purchase_id INTEGER NOT NULL,
            medicine_id INTEGER NOT NULL,

            quantity INTEGER NOT NULL,
            used_qty INTEGER DEFAULT 0,
            returned_qty INTEGER DEFAULT 0,
            purchase_price REAL NOT NULL,
            expiry_date TEXT NOT NULL,

            total REAL NOT NULL,

            return_reason TEXT CHECK(return_reason IN (
                'wrong_medicine',
                'Expired',
                'damaged_package',
                'other'
            )),

            FOREIGN KEY(purchase_id)
                REFERENCES purchase_invoices(id)
                ON DELETE CASCADE
                ON UPDATE CASCADE,

            FOREIGN KEY(medicine_id)
                REFERENCES medicines(id)
                ON DELETE RESTRICT
                ON UPDATE CASCADE
        )
    )")) {
        qDebug() << "Create purchase_items Error:" << q.lastError().text();
    }

    // ================= INVOICE ITEMS =================
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS invoice_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,

            invoice_id INTEGER NOT NULL,
            medicine_id INTEGER NOT NULL,

            quantity INTEGER NOT NULL,
            returned_qty INTEGER DEFAULT 0,

            purchase_price REAL NOT NULL,
            sale_price REAL NOT NULL,

            total REAL NOT NULL,

            batch_id INTEGER,

            return_reason TEXT CHECK(return_reason IN (
                'wrong_medicine',
                'customer_changed_mind',
                'damaged_package',
                'other'
            )),

            FOREIGN KEY(invoice_id)
                REFERENCES invoices(id)
                ON DELETE CASCADE
                ON UPDATE CASCADE,

            FOREIGN KEY(medicine_id)
                REFERENCES medicines(id)
                ON DELETE RESTRICT
                ON UPDATE CASCADE,

            FOREIGN KEY(batch_id)
                REFERENCES purchase_items(id)
                ON DELETE SET NULL
                ON UPDATE CASCADE
        )
    )")) {
        qDebug() << "Create invoice_items Error:" << q.lastError().text();
    }
}

