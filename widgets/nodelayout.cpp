/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "nodelayout.h"

#include <QPair>
#include <QMap>

namespace Plasma
{
NodeLayout::NodeCoordinate::NodeCoordinate(qreal xRelative, qreal yRelative, qreal xAbsolute, qreal yAbsolute)
    : xr(xRelative), xa(xAbsolute), yr(yRelative), ya(yAbsolute) {}

NodeLayout::NodeCoordinate NodeLayout::NodeCoordinate::simple(qreal x, qreal y,
        CoordinateType xType, CoordinateType yType)
{
    NodeLayout::NodeCoordinate coo;
    switch (xType) {
    case Relative:
        coo.xr = x;
        coo.xa = 0;
        break;
    case Absolute:
        coo.xr = 0;
        coo.xa = x;
        break;
    case InnerRelative:
        coo.xr = x;
        coo.xa = INFINITY;
        break;
    }

    switch (yType) {
    case Relative:
        coo.yr = y;
        coo.ya = 0;
        break;
    case Absolute:
        coo.yr = 0;
        coo.ya = y;
        break;
    case InnerRelative:
        coo.yr = y;
        coo.ya = INFINITY;
        break;
    }
    return coo;
}

class NodeLayout::Private {
public:
    QMap <LayoutItem * , QPair < NodeCoordinate, NodeCoordinate > > items;
    QRectF geometry;
    QSizeF sizeHint;

    qreal calculateXPosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        return parentGeometry.left() + (coo.xr * parentGeometry.width())  + coo.xa;
    }

    qreal calculateYPosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        return parentGeometry.top() + (coo.yr * parentGeometry.height())  + coo.ya;
    }

    QPointF calculatePosition(const NodeCoordinate & coo, const QRectF & parentGeometry) const
    {
        return QPointF(
            calculateXPosition(coo, geometry),
            calculateYPosition(coo, geometry)
        );
    }


    QRectF calculateRectangle(LayoutItem * item, QRectF geometry = QRectF()) const
    {
        if (geometry == QRectF()) geometry = this->geometry;

        QRectF result;
        if (!item || !items.contains(item)) return QRectF();

        result.setTopLeft(calculatePosition(items[item].first, geometry));

        if (items[item].second.xa != INFINITY) {
            result.setRight(calculateXPosition(items[item].second, geometry));
        } else {
            result.setWidth(item->sizeHint().width());
            result.moveLeft(result.left() - items[item].second.xr * result.width());
        }

        if (items[item].second.ya != INFINITY) {
            result.setBottom(calculateYPosition(items[item].second, geometry));
        } else {
            result.setHeight(item->sizeHint().height());
            result.moveTop(result.top() - items[item].second.yr * result.height());
        }

        return result;
    }

    void invalidate()
    {
        foreach (LayoutItem * item, items.keys()) {
            if (item) {
                item->setGeometry(calculateRectangle(item));
            }
        }
    }

    void calculateSizeHint(LayoutItem * item = NULL) {
        if (item == NULL) {
            // Recalculate the sizeHint using all items
            sizeHint = QSizeF();
            foreach (LayoutItem * item, items.keys()) {
                if (item) {
                    calculateSizeHint(item);
                }
            }
        } else {
            // Calculate size hint for current item
            QRectF scaled = calculateRectangle(item, QRectF(0, 0, 1, 1));

            // qMin(..., 1.0) so that for autosized elements we don't get smaller
            // size than the item's size itself. The sizeHint for NodeLayout can
            // not do anything smarter concerning the sizeHint when there are
            // autosized elements.

            qreal width  = item->sizeHint().width()  / qMin(scaled.width(), 1.0);
            qreal height = item->sizeHint().height() / qMin(scaled.height(), 1.0);

            if (width > sizeHint.width())   sizeHint.setWidth(width);
            if (height > sizeHint.height()) sizeHint.setHeight(height);
        }
    }

};


NodeLayout::NodeLayout(LayoutItem * parent) 
  : Layout(parent), d(new Private())
{
}

NodeLayout::~NodeLayout()
{
    delete d;
}

Qt::Orientations NodeLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

QRectF NodeLayout::geometry() const
{
    return d->geometry;
}

void NodeLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        return;
    }

    d->geometry = geometry;
    d->invalidate();
}

QSizeF NodeLayout::sizeHint() const
{
    return d->sizeHint;
}

void NodeLayout::addItem (LayoutItem * item)
{
    NodeLayout::addItem (item, NodeCoordinate());
}

void NodeLayout::addItem (LayoutItem * item, NodeCoordinate topLeft, NodeCoordinate bottomRight)
{
    if (!item) return;
    d->items[item] = QPair<NodeCoordinate, NodeCoordinate>(topLeft, bottomRight);
    d->calculateSizeHint(item);
}

void NodeLayout::addItem (LayoutItem * item, NodeCoordinate node, qreal xr, qreal yr)
{
    if (!item) return;
    d->items[item] = QPair<NodeCoordinate, NodeCoordinate>(node,
        NodeCoordinate::simple(xr, yr, NodeCoordinate::InnerRelative, NodeCoordinate::InnerRelative));
    d->calculateSizeHint(item);
}

void NodeLayout::removeItem (LayoutItem * item)
{
    if (!item) return;
    d->items.remove(item);
    d->calculateSizeHint();
}

int NodeLayout::count() const
{
    return d->items.count();
}

int NodeLayout::indexOf(LayoutItem * item) const
{
    if (!item) return -1;
    return d->items.keys().indexOf(item);
}

LayoutItem * NodeLayout::itemAt(int i) const
{
    return d->items.keys()[i];
}

LayoutItem * NodeLayout::takeAt(int i)
{
    LayoutItem * item = itemAt(i);
    removeItem(item);
    return item;
}

}
