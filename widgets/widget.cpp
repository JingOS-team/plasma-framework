/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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


#include "widget.h"

#include <cmath>
#include <limits>

#include <QtCore/QList>
#include <QPainter>
#include <QPixmapCache>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include "layout.h"
#include "plasma/plasma.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : minimumSize(0,0)
            , maximumSize(std::numeric_limits<qreal>::infinity(),
                          std::numeric_limits<qreal>::infinity())
            , parent(0)
            , opacity(1.0)
            , cachePaintMode(Widget::NoCacheMode)
        { }
        ~Private() { }

        QSizeF size;
        QSizeF minimumSize;
        QSizeF maximumSize;

        Widget *parent;
        QList<Widget *> childList;

        qreal opacity;
 
        // Replace with CacheMode in 4.4
#if QT_VERSION >= 0x040400
#warning Replace Plasma::Widget::CachePaintMode with QGraphicsItem::CacheMode
#endif
        Widget::CachePaintMode cachePaintMode; 
        QSize cacheSize;
        QString cacheKey;
        QRectF cacheInvalidated;

        bool shouldPaint(QPainter *painter, const QTransform &transform);
};

QGraphicsItem* Widget::graphicsItem()
{
    return this;
}

bool Widget::Private::shouldPaint(QPainter *painter, const QTransform &transform)
{
    qreal zoomLevel = painter->transform().m11() / transform.m11();
    //return (fabs(zoomLevel - scalingFactor(Plasma::DesktopZoom))) < std::numeric_limits<double>::epsilon();
    return true;
}

Widget::Widget(QGraphicsItem *parent,QObject* parentObject)
  : QObject(parentObject),
    QGraphicsItem(parent),
    d(new Private)
{
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setCachePaintMode(DeviceCoordinateCacheMode);
    d->parent = dynamic_cast<Widget *>(parent);

    if (d->parent) {
        d->parent->addChild(this);
        d->parent->updateGeometry();
    }
}

Widget::~Widget()
{
    delete d;
}

void Widget::setOpacity(qreal opacity)
{
    d->opacity = opacity;
}
qreal Widget::opacity() const
{
    return d->opacity;
}

void Widget::setCachePaintMode(CachePaintMode mode, const QSize &size)
{
    d->cachePaintMode = mode;
    if (mode == NoCacheMode) {
	QPixmapCache::remove(d->cacheKey);
	d->cacheKey.clear();
    } else {
	d->cacheKey = QString("%1").arg(long(this));
	if (mode == ItemCoordinateCacheMode)
	    d->cacheSize = size.isNull() ? boundingRect().size().toSize() : size;
    }
}

Widget::CachePaintMode Widget::cachePaintMode() const
{
    return d->cachePaintMode;
}

void Widget::update(const QRectF &rect)
{
    if (d->cachePaintMode != NoCacheMode)
	d->cacheInvalidated |= rect.isNull() ? boundingRect() : rect;
    QGraphicsItem::update(rect);
}

Qt::Orientations Widget::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void Widget::setMinimumSize(const QSizeF& size)
{
    d->minimumSize = size;
}

QSizeF Widget::minimumSize() const
{
    return d->minimumSize;
}

void Widget::setMaximumSize(const QSizeF& size)
{
    d->maximumSize = size;
}

QSizeF Widget::maximumSize() const
{
    return d->maximumSize;
}

bool Widget::hasHeightForWidth() const
{
    return false;
}

qreal Widget::heightForWidth(qreal w) const
{
    Q_UNUSED(w);

    return -1.0;
}

bool Widget::hasWidthForHeight() const
{
    return false;
}

qreal Widget::widthForHeight(qreal h) const
{
    Q_UNUSED(h);

    return -1.0;
}

QRectF Widget::geometry() const
{
    return QRectF(pos(),d->size);
}

#if 0
QRectF Widget::localGeometry() const
{
    return QRectF(QPointF(0.0f, 0.0f), boundingRect().size);
}
#endif

void Widget::setGeometry(const QRectF& geometry)
{
    if ( d->size != geometry.size() ) {
        prepareGeometryChange();
        qreal width = qBound(d->minimumSize.width(), geometry.size().width(), d->maximumSize.width());
        qreal height = qBound(d->minimumSize.height(), geometry.size().height(), d->maximumSize.height());

        d->size = QSizeF(width, height);

        if ( layout() ) 
            layout()->setGeometry(boundingRect());
    }

    setPos(geometry.topLeft() - boundingRect().topLeft());

    update();
}

void Widget::updateGeometry()
{
    if ( managingLayout() ) {
        managingLayout()->invalidate();
    }
}

QSizeF Widget::sizeHint() const
{
    if (layout()) {
        return layout()->sizeHint();
    } else {
        return QSizeF();
    }
}

QSizeF Widget::size() const
{
    return geometry().size();
}
QRectF Widget::boundingRect() const
{
    return QRectF(QPointF(0,0),geometry().size()); 
}

void Widget::resize(const QSizeF& size)
{
    setGeometry(QRectF(pos(), size));
}

void Widget::resize(qreal w, qreal h)
{
    resize(QSizeF(w, h));
}

Widget *Widget::parent() const
{
    return d->parent;
}

void Widget::addChild(Widget *w)
{
    if (!w) {
        return;
    }

    w->reparent(this);
    d->childList.append(w);

    qDebug("Added Child Widget : %p", (void*)w);

    if (layout()) {
        layout()->addItem(w);
        updateGeometry();
    }
}

void Widget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setOpacity(d->opacity);

    if (d->shouldPaint(painter, transform())) {
	if (d->cachePaintMode == NoCacheMode) {
	    paintWidget(painter, option, widget);
	} else {
	    QRectF brect = boundingRect();
	    QRect boundingRectInt = brect.toRect();

	    // Fetch the off-screen transparent buffer and exposed area info.
	    QPixmap pix;
	    QPixmapCache::find(d->cacheKey, pix);
	    QRectF exposed = d->cacheInvalidated;

	    // Render using item coodinate cache mode.
	    if (d->cachePaintMode == ItemCoordinateCacheMode) {
		// Recreate the pixmap if it's gone.
		if (pix.isNull()) {
		    pix = QPixmap(d->cacheSize);
		    pix.fill(Qt::transparent);
		    exposed = brect;
		}
        
		// Check for newly invalidated areas.
		if (!exposed.isNull()) {
		    d->cacheInvalidated = QRectF();

		    QStyleOptionGraphicsItem cacheOption = *option;
		    cacheOption.exposedRect = exposed.toRect(); // <- truncation

		    QPainter pixmapPainter(&pix);
		    // Fit the item's bounding rect into the pixmap's coordinates.
		    pixmapPainter.scale(pix.width() / brect.width(),
					pix.height() / brect.height());
		    pixmapPainter.translate(-brect.topLeft());

		    // Re-render the invalidated areas of the pixmap. Important: don't
		    // fool the item into using the widget - pass 0 instead of \a
		    // widget.
		    paintWidget(&pixmapPainter, &cacheOption, 0);
		    pixmapPainter.end();

		    // Reinsert this pixmap into the cache
		    QPixmapCache::insert(d->cacheKey, pix);
		}

		// Redraw the exposed area using the transformed painter. Depending on
		// the hardware, this may be a server-side operation, or an expensive
		// qpixmap-image-transform-pixmap roundtrip.
		painter->drawPixmap(brect, pix, QRectF(QPointF(), pix.size()));
		return;
	    }

	    // Render using device coordinate cache mode.
	    if (d->cachePaintMode == DeviceCoordinateCacheMode) {
		QTransform transform = painter->worldTransform();
		QRect deviceRect = transform.mapRect(brect).toRect();

		// Auto-adjust the pixmap size.
		if (deviceRect.size() != pix.size()) {
		    pix = QPixmap(deviceRect.size());
		    pix.fill(Qt::transparent);
		    exposed = brect;
		}

		// Check for newly invalidated areas.
		if (!exposed.isNull()) {
		    d->cacheInvalidated = QRectF();

		    // Construct the new styleoption, reset the exposed rect.
		    QStyleOptionGraphicsItem cacheOption = *option;
		    cacheOption.exposedRect = exposed.toRect(); // <- truncation

		    QPointF viewOrigo = transform.map(QPointF(0,  0));
		    QPointF offset = viewOrigo - deviceRect.topLeft();

		    // Transform the painter, and render the item in device coordinates.
		    QPainter pixmapPainter(&pix);
		    pixmapPainter.translate(offset);
		    pixmapPainter.setWorldTransform(transform, true);
		    pixmapPainter.translate(transform.inverted().map(QPointF(0, 0)));
		    paintWidget(&pixmapPainter, &cacheOption, 0);
		    pixmapPainter.end();

		    // Reinsert this pixmap into the cache
		    QPixmapCache::insert(d->cacheKey, pix);
		}

		// Redraw the exposed area using an untransformed painter. This
		// effectively becomes a bitblit that does not transform the cache.
		painter->setWorldTransform(QTransform());
		painter->drawPixmap(deviceRect.topLeft(), pix);
		return;
	    }
	}
    }
    return;
}

void Widget::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Replaced by widget's own function
}

void Widget::reparent(Widget *w)
{
    d->parent = w;
    setParentItem(w);
    update();
}

} // Plasma namespace

