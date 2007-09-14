/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
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

#include "layoutitem.h"

#include <KDebug>

#include "layout.h"

namespace Plasma
{

class LayoutItem::Private
{
    public:
        Private()
            : layout(0),
              managingLayout(0)
        {
        }

        ~Private() {}

        Layout* layout;
        Layout* managingLayout;
};


LayoutItem::LayoutItem()
    : d(new Private())
{
}

LayoutItem::~LayoutItem()
{
    if (d->managingLayout) {
        d->managingLayout->removeItem(this);
    }

    delete d->layout;
    delete d;
}

QGraphicsItem* LayoutItem::graphicsItem() 
{
    return 0;
}

bool LayoutItem::hasHeightForWidth() const
{
    return false;
}

qreal LayoutItem::heightForWidth(qreal w) const
{
    return 0.0;
}

bool LayoutItem::hasWidthForHeight() const
{
    return false;
}

qreal LayoutItem::widthForHeight(qreal h) const
{
    return 0.0;
}

void LayoutItem::setLayout(Layout* layout)
{
    if (d->layout && layout) {
        kDebug() << " already have a layout.";
        return;
    }

    d->layout = layout;

    if (layout) {
        layout->setParent(this);
    } else {
        // FIXME: we had a layout, but now it has been removed
        // and we are without layout; we should tell our
        // children about this, but LayoutItem doesn't know
        // about children =/
        kDebug() << "layout removed from under us. expect crashes";
    }
}

Layout* LayoutItem::layout() const
{
    return d->layout;
}

void LayoutItem::setManagingLayout(Layout* layout)
{
    if (d->managingLayout) {
        d->managingLayout->removeItem(this);
    }

    d->managingLayout = layout;
}

void LayoutItem::unsetManagingLayout(Layout* layout)
{
    if (d->managingLayout == layout) {
        d->managingLayout = 0;
    }
}

Layout* LayoutItem::managingLayout() const
{
    return d->managingLayout;
}

}
