/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "pushbutton.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QDir>
#include <QApplication>

#include <KIcon>
#include <KIconEffect>
#include <KMimeType>
#include <KPushButton>

#include "theme.h"
#include "svg.h"
#include "panelsvg.h"
#include "animator.h"

namespace Plasma
{

class PushButtonPrivate
{
public:
    PushButtonPrivate(PushButton *pushButton)
        : q(pushButton),
          background(0),
          activeBackgroundPixmap(0),
          animId(0),
          svg(0)
    {
    }

    ~PushButtonPrivate()
    {
        delete svg;
    }

    void setPixmap(PushButton *q)
    {
        if (imagePath.isEmpty()) {
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml")) {
            svg = new Svg();
            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            pm = QPixmap(absImagePath);
        }

        static_cast<KPushButton*>(q->widget())->setIcon(KIcon(pm));
    }

    void renderActiveBackgroundPixmap();
    void syncActiveRect();
    void syncBorders();
    void elementAnimationFinished(int id);

    PushButton *q;

    PanelSvg *background;
    QPixmap *activeBackgroundPixmap;
    int animId;
    QRectF activeRect;

    QString imagePath;
    QString absImagePath;
    Svg *svg;
};


void PushButtonPrivate::renderActiveBackgroundPixmap()
{
    background->setElementPrefix("active");

    activeBackgroundPixmap = new QPixmap(activeRect.size().toSize());
    activeBackgroundPixmap->fill(Qt::transparent);

    QPainter painter(activeBackgroundPixmap);
    background->paintPanel(&painter);
}

void PushButtonPrivate::syncActiveRect()
{
    background->setElementPrefix("normal");

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);

    background->setElementPrefix("active");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    background->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    activeRect = QRectF(QPointF(0,0), q->size());
    activeRect.adjust(left - activeLeft, top - activeTop, -(right - activeRight), -(bottom - activeBottom));

    background->setElementPrefix("normal");
}

void PushButtonPrivate::syncBorders()
{
    //set margins from the normal element
    qreal left, top, right, bottom;

    background->setElementPrefix("normal");
    background->getMargins(left, top, right, bottom);
    q->setContentsMargins(left, top, right, bottom);

    //calc the rect for the over effect
    syncActiveRect();
}

void PushButtonPrivate::elementAnimationFinished(int id)
{
    if (id == animId) {
        animId = -1;
    }
}



PushButton::PushButton(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new PushButtonPrivate(this))
{
    KPushButton* native = new KPushButton;
    connect(native, SIGNAL(clicked()), this, SIGNAL(clicked()));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);

    d->background = new PanelSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedPanels(true);
    d->background->setElementPrefix("normal");
    d->syncBorders();
    setAcceptHoverEvents(true);
    connect(Plasma::Animator::self(), SIGNAL(elementAnimationFinished(int)), this, SLOT(elementAnimationFinished(int)));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(syncBorders()));
}

PushButton::~PushButton()
{
    delete d;
}

void PushButton::setText(const QString &text)
{
    static_cast<KPushButton*>(widget())->setText(text);
}

QString PushButton::text() const
{
    return static_cast<KPushButton*>(widget())->text();
}

void PushButton::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() && 
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            (path[0] == '/' || path.startsWith(":/"))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap(this);
}

QString PushButton::image() const
{
    return d->imagePath;
}

void PushButton::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString PushButton::styleSheet()
{
    return widget()->styleSheet();
}

KPushButton* PushButton::nativeWidget() const
{
    return static_cast<KPushButton*>(widget());
}

void PushButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap(this);

   if (d->background) {
        //resize all four panels
        d->background->setElementPrefix("normal");
        d->background->resizePanel(size());
        d->background->setElementPrefix("pressed");
        d->background->resizePanel(size());
        d->background->setElementPrefix("focus");
        d->background->resizePanel(size());

        d->syncActiveRect();

        d->background->setElementPrefix("active");
        d->background->resizePanel(d->activeRect.size());

        d->background->setElementPrefix("normal");

        if (d->activeBackgroundPixmap) {
            delete d->activeBackgroundPixmap;
            d->activeBackgroundPixmap = 0;
        }
   }

    QGraphicsProxyWidget::resizeEvent(event);
}

void PushButton::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    if (!styleSheet().isNull()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QPixmap *bufferPixmap = 0;

    //Normal button, pressed or not
    if (isEnabled()) {
        if (nativeWidget()->isDown()) {
            d->background->setElementPrefix("pressed");
        } else {
            d->background->setElementPrefix("normal");
        }
        d->background->paintPanel(painter);
    //flat or disabled
    } else if (!isEnabled() || nativeWidget()->isFlat()) {
        bufferPixmap = new QPixmap(rect().size().toSize());
        bufferPixmap->fill(Qt::transparent);

        QPainter buffPainter(bufferPixmap);
        d->background->paintPanel(&buffPainter);
        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        buffPainter.fillRect(bufferPixmap->rect(), QColor(0,0,0,128));

        painter->drawPixmap( 0, 0, *bufferPixmap);
    }

    //if is under mouse draw the animated glow overlay
    if (!nativeWidget()->isDown() && isEnabled() && acceptHoverEvents()) {
        if (d->animId != -1) {
            painter->drawPixmap(d->activeRect.topLeft(), Plasma::Animator::self()->currentPixmap(d->animId) );
        } else if (isUnderMouse() || nativeWidget()->isDefault()) {
            if (d->activeBackgroundPixmap == 0) {
                d->renderActiveBackgroundPixmap();
            }
            painter->drawPixmap( d->activeRect.topLeft(), *d->activeBackgroundPixmap );
        }
    }

    if (nativeWidget()->hasFocus()) {
        d->background->setElementPrefix("focus");
        d->background->paintPanel(painter);
    }


    painter->setPen(Plasma::Theme::defaultTheme()->color(Theme::ButtonTextColor));


    if (nativeWidget()->isDown()) {
        painter->translate(QPoint(1, 1));
    }

    QRectF rect = contentsRect();

    if (!nativeWidget()->icon().isNull()) {
        QPixmap iconPix = nativeWidget()->icon().pixmap(rect.height(), rect.height());
        if (!isEnabled()) {
            KIconEffect *effect = KIconLoader::global()->iconEffect();
            iconPix = effect->apply(iconPix, KIconLoader::Toolbar, KIconLoader::DisabledState);
        }

        if (option->direction == Qt::LeftToRight) {
            painter->drawPixmap(rect.topLeft(), iconPix);
            rect.adjust(rect.height(), 0, 0, 0);
        } else {
            painter->drawPixmap(rect.topRight() - QPoint(rect.height(), 0), iconPix);
            rect.adjust(0, 0, -rect.height(), 0);
        }
    }

    //if there is not enough room for the text make it to fade out
    QFontMetricsF fm(QApplication::font());
    if (rect.width() < fm.width(nativeWidget()->text())) {
        if (!bufferPixmap) {
            bufferPixmap = new QPixmap(rect.size().toSize());
        }

        bufferPixmap->fill(Qt::transparent);

        QPainter p(bufferPixmap);
        p.setPen(painter->pen());

        // Create the alpha gradient for the fade out effect
        QLinearGradient alphaGradient(0, 0, 1, 0);
        alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        if (option->direction == Qt::LeftToRight) {
            alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
            alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));
            p.drawText(bufferPixmap->rect(), Qt::AlignLeft|Qt::AlignVCenter, nativeWidget()->text() );
        } else {
            alphaGradient.setColorAt(0, QColor(0, 0, 0, 0));
            alphaGradient.setColorAt(1, QColor(0, 0, 0, 255));
            p.drawText(bufferPixmap->rect(), Qt::AlignRight|Qt::AlignVCenter, nativeWidget()->text() );
        }

        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(bufferPixmap->rect(), alphaGradient);

        painter->drawPixmap(rect.topLeft(), *bufferPixmap);
    } else {
        painter->drawText(rect, Qt::AlignCenter, nativeWidget()->text() );
    }
}

void PushButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->animId != -1) {
        Plasma::Animator::self()->stopElementAnimation(d->animId);
    }
    d->animId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::AppearAnimation);

    d->background->setElementPrefix("active");

    if (!d->activeBackgroundPixmap) {
        d->renderActiveBackgroundPixmap();
    }
    Plasma::Animator::self()->setInitialPixmap( d->animId, *d->activeBackgroundPixmap );

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void PushButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->animId != -1) {
        Plasma::Animator::self()->stopElementAnimation(d->animId);
    }
    d->animId = Plasma::Animator::self()->animateElement(this, Plasma::Animator::DisappearAnimation);

    d->background->setElementPrefix("active");

    if (!d->activeBackgroundPixmap) {
        d->renderActiveBackgroundPixmap();
    }
    Plasma::Animator::self()->setInitialPixmap( d->animId, *d->activeBackgroundPixmap );

    QGraphicsProxyWidget::hoverLeaveEvent(event);
}

} // namespace Plasma

#include <pushbutton.moc>

