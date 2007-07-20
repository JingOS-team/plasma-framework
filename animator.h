/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *                 2007 Alexis Ménard <darktears31@gmail.com>
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

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QtCore/QObject>
#include <QtGui/QRegion>
#include <QtGui/QPixmap>

#include <kgenericfactory.h>

#include <plasma/plasma_export.h>
#include <plasma/phase.h>

class QGraphicsItem;

namespace Plasma
{

class PLASMA_EXPORT Animator : public QObject
{
    Q_OBJECT

public:
    explicit Animator(QObject *parent = 0, const QStringList& list = QStringList());
    ~Animator();

    // Parameter definitions
    virtual int framesPerSecond(Plasma::Phase::Animation);
    virtual int framesPerSecond(Plasma::Phase::Movement);
    virtual int framesPerSecond(Plasma::Phase::ElementAnimation);
    virtual Phase::CurveShape curve(Plasma::Phase::Animation);
    virtual Phase::CurveShape curve(Plasma::Phase::Movement);
    virtual Phase::CurveShape curve(Plasma::Phase::ElementAnimation);

    // Element animations
    virtual QPixmap elementAppear(qreal frame, const QPixmap& pixmap);
    virtual QPixmap elementDisappear(qreal frame, const QPixmap& pixmap);

    // Item animations
    virtual void appear(qreal progress, QGraphicsItem* item);
    virtual void disappear(qreal progress, QGraphicsItem* item);
    virtual void frameAppear(qreal progress, QGraphicsItem* item, const QRegion& drawable);
    virtual void activate(qreal progress, QGraphicsItem* item);

    // Item movements
    virtual void slideIn(qreal progress, QGraphicsItem* item, const QPoint &destination);
    virtual void slideOut(qreal progress, QGraphicsItem* item, const QPoint &destination);

    // Rendering
    virtual void renderBackground(QImage& background);

private:
    class Private;
    Private * const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_ANIMATOR(libname, classname) \
        K_EXPORT_COMPONENT_FACTORY(                \
                        plasma_animator_##libname,   \
                        KGenericFactory<classname>("plasma_animator_" #libname))

#endif // multiple inclusion guard
