/*
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1 as QtExtras

Item {
    id: root
    width: 100
    height: 100

    Item {
        id: pageOne
        anchors {
            fill: parent
        }
        Column {
            anchors.fill: parent
            PlasmaComponents.Label {
                width: parent.width
                text: "This is a <i>PlasmaComponent</i>"
            }
//             PlasmaComponents.TabBar {
//                 height: 32
//                 width: parent.width
//             }
            Row {
                height: 96

                QtExtras.QIconItem {
                    icon: "preferences-desktop-icons"
                    width: parent.height
                    height: width
                }

                PlasmaCore.IconItem {
                    source: "configure"
                    width: parent.height
                    height: width
                }

            }
        }
    }


    Component.onCompleted: {
        print("Components Test Applet loaded")
    }
}