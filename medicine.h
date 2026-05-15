#ifndef MEDICINE_H
#define MEDICINE_H

#include <QString>
#include <QList>
#include <QDateTime>

class Medicine
{
private:
    int id;
    QString name;
    QString category;
    double sale_price;
    int min_quantity;
    QString barcode;

public:
    Medicine();
    Medicine(int id, QString name, QString category,
             double sale_price ,int min_quantity, QString barcode);

    bool add();
    bool update();
    // QString getStatus() const;
    static QList<Medicine> getAll();
    static QList<Medicine> search(const QString &text);

    enum FilterType {
        ALL,
        AVAILABLE,
        LOW,
        EXPIRED
    };

    static QList<Medicine> filter(Medicine::FilterType filter);

    static Medicine getById(int id);

    QString getName() const;
    QString getCategory() const;
    QString getBarcode() const;
    double getSalePrice() const;
    int getId() const;
    int getMinQuantity() const;

};

#endif