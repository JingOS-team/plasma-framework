/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "extenderapplet_p.h"

#include "extender.h"
#include "extenderitem.h"

#include <KDebug>
#include <KIcon>

#include <QGraphicsLinearLayout>

ExtenderApplet::ExtenderApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ExtenderApplet::~ExtenderApplet()
{
}

void ExtenderApplet::init()
{
    new Plasma::Extender(this);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
    setLayout(layout);
    extender()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    connect(extender(), SIGNAL(geometryChanged()), this, SLOT(adjustSize()));
    connect(extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
            this, SLOT(itemDetached(Plasma::ExtenderItem*)));
    layout->addItem(extender());
    adjustSize();
}

void ExtenderApplet::adjustSize()
{
    layout()->updateGeometry();

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    setPreferredSize(QSizeF(left + right + layout()->preferredWidth(),
                            top + bottom + layout()->preferredHeight()));
    setMinimumSize(  QSizeF(left + right + layout()->minimumWidth(),
                            top + bottom + layout()->minimumHeight()));
    setMaximumSize(  QSizeF(left + right + layout()->maximumWidth(),
                            top + bottom + layout()->maximumHeight()));

    updateGeometry();
    //This wasn't necesarry before... but it is now. weirdness.
    resize(size().width(), preferredHeight());
    layout()->invalidate();
}

void ExtenderApplet::itemDetached(Plasma::ExtenderItem*)
{
    kDebug() << "item detached: " << extender()->attachedItems().count();
    if (!extender()->attachedItems().count()) {
        destroy();
        //FIXME: only fire the signal when the item is really detached, not just being dragged away.
        kDebug() << "delete the extender applet...";
    }
}


#include "extenderapplet_p.moc"

