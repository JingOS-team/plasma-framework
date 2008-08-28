/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
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

#ifndef PLASMA_TOOLTIP_P_H
#define PLASMA_TOOLTIP_P_H

#include <QWidget> // base class

#include <plasma/tooltipmanager.h> //ToolTipContent struct

namespace Plasma {

class ToolTipPrivate;

class ToolTip : public QWidget
{
    Q_OBJECT

public:
    ToolTip(QObject *source);
    ~ToolTip();

    void updateTheme();
    void setContent(const ToolTipManager::ToolTipContent &data);
    void prepareShowing();
    void setActivated(bool value);
    bool isActivated();

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    void sourceDestroyed();

private:
    ToolTipPrivate * const d;
};

} // namespace Plasma

#endif // PLASMA_TOOLTIP_P_H

