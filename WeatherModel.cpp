#include "WeatherModel.h"

WeatherModel::WeatherModel(QObject* parent) : QAbstractListModel(parent)
{
}
void WeatherModel::clear() {
    if (_items.count() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        _items.clear();
        endRemoveRows();
    }
}
void WeatherModel::addItems (const QList<WeatherItem>& items)
{
    clear();
    for (QList<WeatherItem>::const_iterator i = items.begin(); i < items.end(); ++i) {
        _items.append(*i);
    }
    beginInsertRows (QModelIndex(), 0, rowCount() - 1);
    endInsertRows();
}

QVariant WeatherModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= _items.count())
        return QVariant();
    const WeatherItem &item = _items[index.row()];
    switch (role) {
        case DateRole:          return item.date();
        case DescriptionRole:   return item.description();
        case ImageUrlRole:      return item.imageurl();
        case TempRole:          return item.temp();
        case RainRole:          return item.rain();
        case SnowRole:          return item.snow();
        case WindRole:          return item.wind();
        case HumidityRole:      return item.humidity();
    }
    return QVariant();
}

QHash<int, QByteArray> WeatherModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DateRole] = "date";
    roles[DescriptionRole] = "description";
    roles[ImageUrlRole] = "imageurl";
    roles[TempRole] = "temp";
    roles[RainRole] = "rain";
    roles[SnowRole] = "snow";
    roles[WindRole] = "wind";
    roles[HumidityRole] = "humidity";
    return roles;
}

