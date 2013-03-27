/*
    Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "DeclarativeDragArea.h"
#include "DeclarativeMimeData.h"

#include <QDrag>
#include <QIcon>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>
#include <QQmlContext>
#include <QQuickWindow>

#include <KMimeType>

#include <QDebug>

/*!
    A DragArea is used to make an item draggable.
*/

DeclarativeDragArea::DeclarativeDragArea(QQuickItem *parent)
    : QQuickItem(parent),
    m_delegate(0),
    m_source(0),
    m_target(0),
    m_enabled(true),
    m_draggingJustStarted(false),
    m_supportedActions(Qt::MoveAction),
    m_defaultAction(Qt::MoveAction),
    m_data(new DeclarativeMimeData())    // m_data is owned by us, and we shouldn't pass it to Qt directly as it will automatically delete it after the drag and drop.
{
    m_startDragDistance = QApplication::startDragDistance();
    setAcceptedMouseButtons(Qt::LeftButton);
    //setFiltersChildEvents(true);
    setFiltersChildMouseEvents(true);
}

DeclarativeDragArea::~DeclarativeDragArea()
{
    if (m_data) {
        delete m_data;
    }
}

/*!
  The delegate is the item that will be displayed next to the mouse cursor during the drag and drop operation.
  It usually consists of a large, semi-transparent icon representing the data being dragged.
*/
QQuickItem* DeclarativeDragArea::delegate() const
{
    return m_delegate;
}

void DeclarativeDragArea::setDelegate(QQuickItem *delegate)
{
    if (m_delegate != delegate) {
        qDebug() << " ______________________________________________ " << delegate;
        m_delegate = delegate;
        emit delegateChanged();
    }
}
void DeclarativeDragArea::resetDelegate()
{
    setDelegate(0);
}

/*!
  The QML element that is the source of this drag and drop operation. This can be defined to any item, and will
  be available to the DropArea as event.data.source
*/
QQuickItem* DeclarativeDragArea::source() const
{
    return m_source;
}

void DeclarativeDragArea::setSource(QQuickItem* source)
{
    if (m_source != source) {
        m_source = source;
        emit sourceChanged();
    }
}

void DeclarativeDragArea::resetSource()
{
    setSource(0);
}

// target
QQuickItem* DeclarativeDragArea::target() const
{
    //TODO: implement me
    return 0;
}

// data
DeclarativeMimeData* DeclarativeDragArea::mimeData() const
{
    return m_data;
}

// startDragDistance
int DeclarativeDragArea::startDragDistance() const
{
    return m_startDragDistance;
}

void DeclarativeDragArea::setStartDragDistance(int distance)
{
    if (distance == m_startDragDistance) {
        return;
    }

    m_startDragDistance = distance;
    emit startDragDistanceChanged();
}

// delegateImage
QVariant DeclarativeDragArea::delegateImage() const
{
    return m_delegateImage;
}

void DeclarativeDragArea::setDelegateImage(const QVariant &image)
{
    if (image.canConvert<QImage>() && image.value<QImage>() == m_delegateImage) {
        return;
    }

    if (image.canConvert<QImage>()) {
        m_delegateImage = image.value<QImage>();
    } else {
        m_delegateImage = image.value<QIcon>().pixmap(QSize(48, 48)).toImage();
    }

    emit delegateImageChanged();
}

// enabled
bool DeclarativeDragArea::isEnabled() const
{
    return m_enabled;
}
void DeclarativeDragArea::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

// supported actions
Qt::DropActions DeclarativeDragArea::supportedActions() const
{
    return m_supportedActions;
}
void DeclarativeDragArea::setSupportedActions(Qt::DropActions actions)
{
    if (actions != m_supportedActions) {
        m_supportedActions = actions;
        emit supportedActionsChanged();
    }
}

// default action
Qt::DropAction DeclarativeDragArea::defaultAction() const
{
    return m_defaultAction;
}
void DeclarativeDragArea::setDefaultAction(Qt::DropAction action)
{
    if (action != m_defaultAction) {
        m_defaultAction = action;
        emit defaultActionChanged();
    }
}

void DeclarativeDragArea::mousePressEvent(QMouseEvent* event)
{
    m_buttonDownPos = event->screenPos();
    m_draggingJustStarted = true;
}


void DeclarativeDragArea::mouseMoveEvent(QMouseEvent *event)
{
    if ( !m_enabled
         || QLineF(event->screenPos(), m_buttonDownPos).length()
            < m_startDragDistance && false) {
        qDebug() << "return";
        return;
    }
    if (m_draggingJustStarted) {
        m_draggingJustStarted = false;

        QDrag *drag = new QDrag(parent());
        DeclarativeMimeData* dataCopy = new DeclarativeMimeData(m_data); //Qt will take ownership of this copy and delete it.
        drag->setMimeData(dataCopy);

        QSize _s(48,48);

        if (!m_delegateImage.isNull()) {
            drag->setPixmap(QPixmap::fromImage(m_delegateImage));
            qDebug() << "++++++isntnull";
        } else {
            if (m_delegate) {
                QRectF rf;
                qDebug() << "has delegate" << m_delegate;
                rf = QRectF(0, 0, m_delegate->width(), m_delegate->height());
                rf = m_delegate->mapRectToScene(rf);
                QImage grabbed = window()->grabWindow();
                //rf = rf.intersected(QRectF(0, 0, grabbed.width(), grabbed.height()));
                grabbed = grabbed.copy(rf.toAlignedRect());
                qDebug() << " +++++++++++++++++++++++ dim: " << rf;
                grabbed.save("file:///tmp/grabbed.png");
                QPixmap pm = QPixmap::fromImage(grabbed);
                qDebug() << " set new pixmap" << grabbed.size();
                drag->setPixmap(pm);

            } else if (mimeData()->hasImage()) {
//                 rf = QRectF(x(), y(), width(), height());
//                 rf = mapRectToScene(rf);
                QImage im = qvariant_cast<QImage>(mimeData()->imageData());
                drag->setPixmap(QPixmap::fromImage(im));
            } else if (mimeData()->hasColor()) {
                QPixmap px(_s);
                px.fill(mimeData()->color());
                drag->setPixmap(px);
            }
        }

        drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2)); // TODO: Make a property for that
        //setCursor(Qt::OpenHandCursor);    //TODO? Make a property for the cursor

        emit dragStarted();

        Qt::DropAction action = drag->exec(m_supportedActions, m_defaultAction);
        emit drop(action);
    }

}

bool DeclarativeDragArea::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        qDebug() << "move in dragarea";
        mouseMoveEvent(me);
    }

    return QQuickItem::childMouseEventFilter(item, event);
}
