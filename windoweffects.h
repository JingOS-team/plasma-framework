/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_WINDOWEFFECTS_H
#define PLASMA_WINDOWEFFECTS_H

#include <QtGui/QWidget>

#include <plasma/plasma.h>

/** @headerfile plasma/windoweffect.h <Plasma/PaintUtils> */

namespace Plasma
{

/**
 * Namespace for all window effects for Plasma/KWin interaction
 * @since 4.4
 */
namespace WindowEffects
{
    enum Effect {
        Slide = 1,
        WindowPreview = 2
    };

    /**
     * @return if an atom property is available
     *
     * @param effect the effect we want to check
     * @since 4.4
     */
    PLASMA_EXPORT bool isEffectAvailable(Effect effect);

    /**
     * Mark a window as sliding from screen edge
     *
     * @arg id of the window on which we want to apply the effect
     * @arg location edge of the screen from which we want the slifing effect.
     *               Desktop and FLoating won't have effect.
     * @arg offset distance in pixels from the screen edge defined by location
     * @since 4.4
     */
    PLASMA_EXPORT void slideWindow(WId id, Plasma::Location location, int offset);

    /**
     * Mark a window as sliding from screen edge
     * This is an overloaded member function provided for convenience
     *
     * @arg widget QWidget corresponding to the top level window we want to animate
     * @arg location edge of the screen from which we want the slifing effect.
     *               Desktop and FLoating won't have effect.
     * @since 4.4
     */
    PLASMA_EXPORT void slideWindow(QWidget *widget, Plasma::Location location);

    /**
     * @return dimension of all the windows passed as parameter
     *
     * @param ids all the windows we want the size
     * @since 4.4
     */
    PLASMA_EXPORT QList<QSize> windowSizes(const QList<WId> &ids);

    /**
     * Paint inside the window parent the thumbnails of the windows list in
     * the respective rectangles of the rects list
     *
     * @param parent window where we should paint
     * @param windows windows we want a thumbnail of.
     *                If it is empty any thumbnail will be deleted
     * @param rects rectangles in parent coordinates where to paint the window thumbnails.
     *              If it is empty any thumbnail will be deleted
     * @since 4.4
     */
    PLASMA_EXPORT void showWindowThumbnails(WId parent, const QList<WId> &windows = QList<WId>(), const QList<QRect> &rects = QList<QRect>());
}

} // namespace Plasma

#endif

