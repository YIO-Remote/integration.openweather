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

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include "yio-interface/entities/weatherinterface.h"

class WeatherModel : public QAbstractListModel {
    Q_OBJECT

 public:
    enum BrowseRoles {
        DateRole = Qt::UserRole + 1,
        DescriptionRole,
        ImageUrlRole,
        TempRole,
        RainRole,
        SnowRole,
        WindRole,
        HumidityRole
    };

    explicit WeatherModel(QObject* parent = nullptr);

    void addItems(const QList<WeatherItem>& items);

    int rowCount(const QModelIndex& parent = QModelIndex()) const {
        Q_UNUSED(parent)
        return _items.count();
    }
    void     clear();
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

 protected:
    QHash<int, QByteArray> roleNames() const;

 private:
    QList<WeatherItem> _items;
};
