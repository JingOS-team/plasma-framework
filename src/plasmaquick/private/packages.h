/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2013 by Marco Martin <mart@kde.org>                             *
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef SHELLPACKAGES_H
#define SHELLPACKAGES_H

#include <Plasma/PackageStructure>
#include <Plasma/Plasma>

class LookAndFeelPackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) Q_DECL_OVERRIDE;
};

class QmlWallpaperPackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) Q_DECL_OVERRIDE;
};

class LayoutTemplatePackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) Q_DECL_OVERRIDE;
};

#endif // LOOKANDFEELPACKAGE_H
