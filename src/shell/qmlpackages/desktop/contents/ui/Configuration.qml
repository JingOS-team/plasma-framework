/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.plasma.core 0.1 as PlasmaCore

//TODO: all of this will be done with desktop components
Rectangle {
    id: root
    color: "lightgray"
    width: 640
    height: 480

    function saveConfig() {
        for (var key in plasmoid.configuration) {
            if (main.item["cfg_"+key] !== undefined) {
                plasmoid.configuration[key] = main.item["cfg_"+key]
            }
        }
    }

    function restoreConfig() {
        for (var key in plasmoid.configuration) {
            if (main.item["cfg_"+key] !== undefined) {
                main.item["cfg_"+key] = plasmoid.configuration[key]
            }
        }
    }

    Component.onCompleted: {
        main.sourceComponent = configDialog.configPages[0].component
        root.restoreConfig()
        root.width = mainColumn.implicitWidth
        root.height = mainColumn.implicitHeight
    }

    property list<QtObject> globalConfigPages: [
        QtObject {
            property string name: "Keyboard shortcuts"
            property string icon: "preferences-desktop-keyboard"
            property Component component: Component {
                Item {
                    id: iconsPage
                    width: childrenRect.width
                    height: childrenRect.height

                    PlasmaComponents.Button {
                        iconSource: "settings"
                        text: "None"
                    }
                }
            }
        }
    ]

    Column {
        id: mainColumn
        anchors.fill: parent
        Row {
            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: 4
            height: parent.height - buttonsRow.height
            PlasmaExtras.ScrollArea {
                id: categoriesScroll
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                width: 100
                Flickable {
                    contentWidth: width
                    contentHeight: categoriesColumn.height
                    Column {
                        id: categoriesColumn
                        width: parent.width
                        Repeater {
                            model: configDialog.configPages.length
                            delegate: ConfigCategoryDelegate {
                                dataSource: configDialog.configPages
                            }
                        }
                        Repeater {
                            model: globalConfigPages.length
                            delegate: ConfigCategoryDelegate {
                                dataSource: globalConfigPages
                            }
                        }
                    }
                }
            }
            PlasmaExtras.ScrollArea {
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    margins: 4
                }
                width: parent.width - categoriesScroll.width - 8
                Flickable {
                    contentWidth: width
                    contentHeight: main.height
                    Item {
                        width: parent.width
                        height: childrenRect.height
                        Loader {
                            id: main
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: childrenRect.width
                            height: childrenRect.height
                        }
                    }
                }
            }
        }
        Row {
            id: buttonsRow
            spacing: 4
            anchors {
                right: parent.right
                rightMargin: spacing
            }
            PlasmaComponents.Button {
                iconSource: "dialog-ok"
                text: "Ok"
                onClicked: {
                    root.saveConfig()
                    configDialog.close()
                }
            }
            PlasmaComponents.Button {
                iconSource: "dialog-ok-apply"
                text: "Apply"
                onClicked: root.saveConfig()
            }
            PlasmaComponents.Button {
                iconSource: "dialog-cancel"
                text: "Cancel"
                onClicked: configDialog.close()
            }
        }
    }
}
