/******************************************************************************
 *
 * Copyright (C) 2019 Christian Riedl <ric@rts.co.at>
 *
 * This file is part of the YIO-Remote software project.
 *
 * YIO-Remote software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YIO-Remote software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YIO-Remote software. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *****************************************************************************/

#include "WeatherModel.h"

WeatherModel::WeatherModel(QObject* parent) : QAbstractListModel(parent) {}
void WeatherModel::clear() {
    if (_items.count() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        _items.clear();
        endRemoveRows();
    }
}
void WeatherModel::addItems(const QList<WeatherItem>& items) {
    clear();
    for (QList<WeatherItem>::const_iterator i = items.begin(); i < items.end(); ++i) {
        _items.append(*i);
    }
    beginInsertRows(QModelIndex(), 0, rowCount() - 1);
    endInsertRows();
}

QVariant WeatherModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= _items.count()) {
        return QVariant();
    }
    const WeatherItem& item = _items[index.row()];
    switch (role) {
        case DateRole:
            return item.date();
        case DescriptionRole:
            return item.description();
        case ImageUrlRole:
            return item.imageurl();
        case TempRole:
            return item.temp();
        case RainRole:
            return item.rain();
        case SnowRole:
            return item.snow();
        case WindRole:
            return item.wind();
        case HumidityRole:
            return item.humidity();
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
