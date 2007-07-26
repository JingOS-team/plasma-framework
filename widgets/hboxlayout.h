/*
 *   Copyright (C) 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#ifndef __H_BOX_LAYOUT__
#define __H_BOX_LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/plasma_export.h>
#include <plasma/widgets/boxlayout.h>

namespace Plasma
{


/**
 * Vertical Box Layout
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * This class implements a Horizontal Box Layout, it just lays items horizontally, from left to right.
 */
class PLASMA_EXPORT HBoxLayout : public BoxLayout
{
	public:

        /**
         * Constructor.
         */
		HBoxLayout(LayoutItem *parent = 0);

        /**
         * Virtual Destructor.
         */
		virtual ~HBoxLayout();

		Qt::Orientations expandingDirections() const;

		bool hasWidthForHeight() const;
		qreal widthForHeight(qreal w) const;

		void setGeometry(const QRectF& geometry);

		QSizeF sizeHint() const;

	private:
		class Private;
		Private *const d;
};

}

#endif /* __H_BOX_LAYOUT__ */
