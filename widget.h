/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef WIDGET
#define WIDGET

#include <QWeakPointer>

#include "applet.h"

#include "../plasmagenericshell_export.h"

namespace Plasma
{
    class Applet;
} // namespace Plasma

class PLASMAGENERICSHELL_EXPORT Widget : public Applet
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QStringList configKeys READ configKeys)
    Q_PROPERTY(QStringList configGroups READ configGroups)
    Q_PROPERTY(int index WRITE setIndex READ index)
    Q_PROPERTY(QRectF geometry WRITE setGeometry READ geometry)
    Q_PROPERTY(QStringList currentConfigGroup WRITE setCurrentConfigGroup READ currentConfigGroup)
    Q_PROPERTY(QString globalShortcut WRITE setGlobalShortcut READ globalShorcut)


public:
    Widget(Plasma::Applet *applet, QObject *parent = 0);
    ~Widget();

    uint id() const;
    QString type() const;

    int index() const;
    void setIndex(int index);

    QRectF geometry() const;
    void setGeometry(const QRectF &geometry);

    void setGlobalShortcut(const QString &shortcut);
    QString globalShorcut() const;

    Plasma::Applet *applet() const;

public Q_SLOTS:
    void remove();
    void showConfigurationInterface();

    // from the applet interface
    QVariant readConfig(const QString &key, const QVariant &def = QString()) const { return Applet::readConfig(key, def); }
    void writeConfig(const QString &key, const QVariant &value) { Applet::writeConfig(key, value); }
    void reloadConfig() { Applet::reloadConfig(); }

private:
    QWeakPointer<Plasma::Applet> m_applet;
};

#endif

