/***********************************************************************/
/* animationprivate.h                                                  */
/*                                                                     */
/* Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*/
/*                                                                     */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public          */
/* License as published by the Free Software Foundation; either        */
/* version 2.1 of the License, or (at your option) any later version.  */
/*                                                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.                     */
/*                                                                     */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA                                                     */
/***********************************************************************/
#ifndef PLASMA_ANIMATIONPRIVATE_H
#define PLASMA_ANIMATIONPRIVATE_H

#include <QEasingCurve>
#include <QWeakPointer>
class QAbstractAnimation;

namespace Plasma
{

class AnimationPrivate
{
public:

    AnimationPrivate();
    /**
     * Object the animation(s) should act upon.
     */
    QWeakPointer<QGraphicsWidget> animObject;

    /**
     * Animation easing curve type
     */
    QEasingCurve easingCurve;

    /**
     * Duration of the animation. Default is 250ms.
     */
    int duration;
};

}

#endif
