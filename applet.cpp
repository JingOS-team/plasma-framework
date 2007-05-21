/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
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

#include "applet.h"

#include <QEvent>
#include <QList>
#include <QSize>
#include <QStringList>
#include <QTimer>

#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>

#include "interface.h"

namespace Plasma
{

class Applet::Private
{
    public:
        Private( KService::Ptr appletDescription, int uniqueID )
            : appletId( uniqueID ),
              globalConfig( 0 ),
              appletConfig( 0 ),
              appletDescription( new KPluginInfo( appletDescription ) )
        { }

        ~Private()
        {
            foreach ( const QString& engine, loadedEngines ) {
               Interface::self()->unloadDataEngine( engine );
            }
            delete appletDescription;
        }

        int appletId;
        KSharedConfig::Ptr globalConfig;
        KSharedConfig::Ptr appletConfig;
        KPluginInfo* appletDescription;
        QList<QObject*> watchedForFocus;
        QStringList loadedEngines;
};

Applet::Applet( QGraphicsItem *parent,
                const QString& serviceID,
                int appletId )
        : QWidget( 0 ),
          QGraphicsItemGroup( parent ),
          d( new Private( KService::serviceByStorageId( serviceID ), appletId ) )
{
}

Applet::~Applet()
{
    needsFocus( false );
    delete d;
}

KSharedConfig::Ptr Applet::appletConfig() const
{
    if ( !d->appletConfig ) {
        QString file = KStandardDirs::locateLocal( "appdata",
                                                   "applets/" + instanceName() + "rc",
                                                   true );
        d->appletConfig = KSharedConfig::openConfig( file );
    }

    return d->appletConfig;
}

KSharedConfig::Ptr Applet::globalAppletConfig() const
{
    if ( !d->globalConfig ) {
        QString file = KStandardDirs::locateLocal( "config", "plasma_" + globalName() + "rc" );
        d->globalConfig = KSharedConfig::openConfig( file );
    }

    return d->globalConfig;
}

bool Applet::loadDataEngine( const QString& name )
{
    if ( d->loadedEngines.indexOf( name ) != -1 ) {
        return true;
    }

    if ( Plasma::Interface::self()->loadDataEngine( name ) ) {
        d->loadedEngines.append( name );
        return true;
    }

    return false;
}

void Applet::constraintsUpdated()
{
}

QString Applet::globalName() const
{
    return d->appletDescription->service()->library();
}

QString Applet::instanceName() const
{
    return d->appletDescription->service()->library() + QString::number( d->appletId );
}

void Applet::watchForFocus(QObject *widget, bool watch)
{
    if ( !widget ) {
        return;
    }

    int index = d->watchedForFocus.indexOf(widget);
    if ( watch ) {
        if ( index == -1 ) {
            d->watchedForFocus.append( widget );
            widget->installEventFilter( this );
        }
    } else if ( index != -1 ) {
        d->watchedForFocus.removeAt( index );
        widget->removeEventFilter( this );
    }
}

void Applet::needsFocus( bool focus )
{
    if ( focus == QWidget::hasFocus() ||
         focus == QGraphicsItem::hasFocus() ) {
        return;
    }

    emit requestFocus(focus);
}

bool Applet::eventFilter( QObject *o, QEvent * e )
{
    if ( !d->watchedForFocus.contains( o ) )
    {
        if ( e->type() == QEvent::MouseButtonRelease ||
             e->type() == QEvent::FocusIn ) {
            needsFocus( true );
        } else if ( e->type() == QEvent::FocusOut ) {
            needsFocus( false );
        }
    }

    return QWidget::eventFilter( o, e );
}

} // Plasma namespace

#include "applet.moc"
