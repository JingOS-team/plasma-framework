/*
 *   Copyright 2007 by Robert Knight <robertknight@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.

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

#include <plasma/plasma_export.h>
#include <plasma/widgets/layout.h>

#include "freelayout.h"

namespace Plasma
{

class FreeLayout::Private
{
public:
    QList<LayoutItem*> children;
    QRectF geometry;
};

FreeLayout::FreeLayout(LayoutItem *parent)
    : Layout(parent),
      d(new Private)
{
}

FreeLayout::~FreeLayout()
{
    delete d;
}

Qt::Orientations FreeLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void FreeLayout::addItem(LayoutItem *item)
{
    d->children << item;
    item->setManagingLayout(this);
}

void FreeLayout::removeItem(LayoutItem *item)
{
    d->children.removeAll(item);
}

int FreeLayout::indexOf(LayoutItem *item) const
{
    return d->children.indexOf(item);
}

LayoutItem * FreeLayout::itemAt(int i) const
{
    return d->children[i];
}

int FreeLayout::count() const
{
    return d->children.count();
}

LayoutItem * FreeLayout::takeAt(int i)
{
    return d->children.takeAt(i);
}

void FreeLayout::setGeometry(const QRectF &geometry)
{
    foreach (LayoutItem *child , d->children) {
        child->setGeometry(QRectF(child->geometry().topLeft(),child->sizeHint()));
    }
    d->geometry = geometry;
}

QRectF FreeLayout::geometry() const
{
    return d->geometry;
}

QSizeF FreeLayout::sizeHint() const
{
    return maximumSize();
}

}

