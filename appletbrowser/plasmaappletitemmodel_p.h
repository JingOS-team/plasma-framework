/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or,
 *   at your option, any later version) as published by the Free Software
 *   Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMAAPPLETSMODEL_H_
#define PLASMAAPPLETSMODEL_H_

#include <KPluginInfo>

#include <plasma/applet.h>

#include "kcategorizeditemsview_p.h"

/**
 * Implementation of the KCategorizedItemsViewModels::AbstractItem
 */
class PlasmaAppletItem : public KCategorizedItemsViewModels::AbstractItem,
        QObject
{
public:
    enum FilterFlag {NoFilter = 0,
        Favorite = 1,
        Used = 2,
        Recommended = 4};

    Q_DECLARE_FLAGS(FilterFlags, FilterFlag)

    PlasmaAppletItem(QObject * parent, QString name, QString pluginName,
            QString description, QString category, QIcon icon,
            FilterFlags flags = NoFilter);

    PlasmaAppletItem(QObject *parent, const KPluginInfo& info,
            FilterFlags flags = NoFilter);

    virtual QString name() const;
    QString pluginName() const;
    virtual QString description() const;
    virtual void setFavorite(bool favorite);
    virtual bool passesFiltering(
            const KCategorizedItemsViewModels::Filter & filter) const;
};

class PlasmaAppletItemModel :
    public KCategorizedItemsViewModels::DefaultItemModel
{
public:
    PlasmaAppletItemModel(QObject * parent = 0);

    QStringList mimeTypes() const;

    QMimeData* mimeData(const QModelIndexList & indexes) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlasmaAppletItem::FilterFlags)

#endif /*PLASMAAPPLETSMODEL_H_*/
