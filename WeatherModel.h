#ifndef WEATHERMODEL_H
#define WEATHERMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "../remote-software/sources/entities/weatherinterface.h"

class WeatherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum BrowseRoles {
        DateRole = Qt::UserRole + 1,
        DescriptionRole, ImageUrlRole, TempRole, RainRole, SnowRole, WindRole, HumidityRole  };

    explicit WeatherModel(QObject* parent = nullptr);

    void addItems       (const QList<WeatherItem>& items);

    int  rowCount       (const QModelIndex& parent = QModelIndex()) const
    {
        Q_UNUSED(parent)
        return _items.count();
    }
    void clear          ();
    QVariant data       (const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<WeatherItem>    _items;
};

#endif // WEATHERMODEL_H
