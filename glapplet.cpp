/*
 *   Copyright 2007 Zack Rusin <zack@kde.org>
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

#include "glapplet.h"

#include <QtOpenGL/QGLPixelBuffer>
#include <QtGui/QPainter>
#include <QtGui/QImage>

namespace Plasma {

class GLApplet::Private
{
public:
    Private()
    {
        init();
    }
    ~Private()
    {
        delete pbuf;
        delete dummy;
    }
    void init()
    {
        dummy = new QGLWidget((QWidget *) 0);
        QGLFormat format = QGLFormat::defaultFormat();
        format.setSampleBuffers(true);
        format.setAlphaBufferSize(8);
        //dummy size construction
        pbuf = new QGLPixelBuffer(300, 300, format, dummy);
        if (pbuf->isValid())
            pbuf->makeCurrent();
    }
    void updateGlSize(const QSize &size)
    {
        if (size.width() > pbuf->width() ||
            size.height() > pbuf->height()) {
            QGLFormat format = pbuf->format();
            delete pbuf;
            pbuf = new QGLPixelBuffer(size, format, dummy);
        }
    }

public:
    QGLPixelBuffer *pbuf;
    QGLWidget      *dummy;
};

GLApplet::GLApplet(QGraphicsItem *parent,
                   const QString &serviceId,
                   int appletId)
    : Applet(parent, serviceId, appletId),
      d(new Private)
{
    if (!d->dummy->isValid() ||
        !d->pbuf->isValid()) {
        setFailedToLaunch(true, i18n("This system does not support OpenGL applets."));
    }
}

GLApplet::GLApplet(QObject *parent, const QVariantList &args)
    : Applet(parent, args),
      d(new Private)
{
}

GLApplet::~GLApplet()
{
    delete d;
}

GLuint GLApplet::bindTexture(const QImage &image, GLenum target)
{
    Q_ASSERT(d->pbuf);
    if (!d->dummy->isValid())
        return 0;
    return d->dummy->bindTexture(image, target);
}

void GLApplet::deleteTexture(GLuint textureId)
{
    Q_ASSERT(d->pbuf);
    d->dummy->deleteTexture(textureId);
}

static inline QPainterPath headerPath(const QRectF &r, int roundness,
                                      int headerHeight=10)
{
    QPainterPath path;
    int xRnd = roundness;
    int yRnd = roundness;
    if (r.width() > r.height())
        xRnd = int(roundness * r.height()/r.width());
    else
        yRnd = int(roundness * r.width()/r.height());

    if(xRnd >= 100)                          // fix ranges
        xRnd = 99;
    if(yRnd >= 100)
        yRnd = 99;
    if(xRnd <= 0 || yRnd <= 0) {             // add normal rectangle
        path.addRect(r);
        return path;
    }

    QRectF rect = r.normalized();

    if (rect.isNull())
        return path;

    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = rect.width();
    qreal h = rect.height();
    qreal rxx = w*xRnd/200;
    qreal ryy = h*yRnd/200;
    // were there overflows?
    if (rxx < 0)
        rxx = w/200*xRnd;
    if (ryy < 0)
        ryy = h/200*yRnd;
    qreal rxx2 = 2*rxx;
    qreal ryy2 = 2*ryy;

    path.arcMoveTo(x, y, rxx2, ryy2, 90);
    path.arcTo(x,        y, rxx2, ryy2, 90, 90);
    QPointF pt = path.currentPosition();
    path.lineTo(x, pt.y()+headerHeight);
    path.lineTo(x+w, pt.y()+headerHeight);
    path.lineTo(x+w, pt.y());
    path.arcTo(x+w-rxx2, y, rxx2, ryy2, 0, 90);
    path.closeSubpath();

    return path;
}

void GLApplet::paintInterface(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              const QRect &contentsRect)
{
    Q_ASSERT(d->pbuf);
    if ((!d->dummy->isValid() ||
         !d->pbuf->isValid())) {
        if (!failedToLaunch()) {
            setFailedToLaunch(true, i18n("Your machine does not support OpenGL applets."));
        }

        return;
    }
    d->pbuf->makeCurrent();

    // handle background filling
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix m = painter->worldMatrix();
    QRect deviceRect = m.mapRect(boundingRect()).toRect();
    d->updateGlSize(deviceRect.size());

    // redirect this widget's painting into the pbuffer
    QPainter p(d->pbuf);
    paintGLInterface(&p, option);

    // draw the pbuffer contents to the backingstore
    QImage image = d->pbuf->toImage();
    painter->drawImage(0, 0, image);
}

void GLApplet::makeCurrent()
{
    if (!d->dummy->isValid() ||
        !d->pbuf->isValid())
        d->dummy->makeCurrent();
}

} // Plasma namespace

#include "glapplet.moc"
