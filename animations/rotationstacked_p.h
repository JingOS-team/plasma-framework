/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file This file contains the definition for the StackedRotationAnimation.
 */

#ifndef PLASMA_ROTATIONSTACKED_P_H
#define PLASMA_ROTATIONSTACKED_P_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

#include <QGraphicsLayoutItem>

class QGraphicsRotation;
class StackedLayout;

namespace Plasma {

/* TODO:
 * create a parent class for rotations
 */
/**
 * @class RotationStackedAnimation plasma/animations/rotationstacked_p.h
 * @short 3D like rotation animation
 * Use this class when you want to rotate a widget along an axis (e.g. Y)
 * and display a 'hidden' widget behind it. See also \ref RotationAnimation.
 */
class RotationStackedAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qint8 movementDirection READ movementDirection WRITE setMovementDirection)
    Q_PROPERTY(QGraphicsLayoutItem* layout READ layout)
    Q_PROPERTY(qint8 reference READ reference WRITE setReference)
    Q_PROPERTY(QGraphicsWidget* backWidget READ backWidget WRITE setBackWidget)

public:
    explicit RotationStackedAnimation(QObject *parent = 0);

    ~RotationStackedAnimation();

    /**
     * Set the animation direction
     * @arg direction animation direction
     */
    void setMovementDirection(const qint8 &direction);

    /**
     * Get the animation direction
     */
    qint8 movementDirection() const;

    /**
     * Set rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     * @arg reference The reference
     */
    void setReference(const qint8 &reference);

    /**
     * Rotation reference (e.g. Center, Up, Down, Left, Right) can
     * be combined (i.e. Center|Up)
     */
    qint8 reference() const;

    /**
     * Get the layout where the widgetToAnimate and backWidget are.
     */
    QGraphicsLayoutItem *layout();

    /**
     * Get the back widget
     */

    QGraphicsWidget *backWidget();

    /**
     * Set the back widget that is used after the animation to be finished
     * @arg backWidget The back widget
     */
    void setBackWidget(QGraphicsWidget *backWidget);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    /**
     * Updates the front and back widget rotation animation transformations,
     * according to the given animation reference point and direction.
     */
    void updateTransformations();

    /** Reference, the default is Center (see \ref Animation::Reference) */
    qint8 m_reference;
    /** Animation direction, the default is MoveUp (see \ref Animation::MovementDirection) */
    MovementDirection m_animDirection;
    /**  Object the animation(s) should act upon. */
    QWeakPointer<QGraphicsWidget> m_backWidget;
    /** Layout where widget would be added */
    QWeakPointer<StackedLayout> m_wLayout;
    /** Back Widget Rotation transform object */
    QGraphicsRotation *m_backRotation;
    /** Front Widget Rotation transform object */
    QGraphicsRotation *m_frontRotation;

    static const int s_sideAngle;
};
} // Plasma

#endif // PLASMA_ROTATIONSTACKED_P_H
