/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "containment.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KMimeType>
#include <KRun>
#include <KServiceTypeTrader>
#include <KStandardDirs>

#include "applethandle_p.h"
#include "corona.h"
#include "phase.h"
#include "toolbox_p.h"
#include "svg.h"

#include "widgets/freelayout.h"
#include "widgets/boxlayout.h"
#include "widgets/pushbutton.h"

namespace Plasma
{

static const int INTER_CONTAINMENT_MARGIN = 6;

class Containment::Private
{
public:
    Private()
        : formFactor(Planar),
          location(Floating),
          screen(0),
          immutable(false),
          toolbox(0)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
    }

    FormFactor formFactor;
    Location location;
    Applet::List applets;
    QMap<Applet*, AppletHandle*> handles;
    int screen;
    bool immutable;
    DesktopToolbox *toolbox;
};

Containment::Containment(QGraphicsItem* parent,
                         const QString& serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new Private)
{
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private)
{
    setDrawStandardBackground(false);
}

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
    setCachePaintMode(NoCacheMode);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);

    //TODO: would be nice to not do this on init, as it causes Phase to init
    connect(Phase::self(), SIGNAL(animationComplete(QGraphicsItem*,Plasma::Phase::Animation)),
            this, SLOT(appletAnimationComplete(QGraphicsItem*,Plasma::Phase::Animation)));
    
    if (type() == DesktopContainment) {
        Plasma::PushButton *tool = new Plasma::PushButton(i18n("Add Widgets"));
        tool->resize(tool->sizeHint());
        addToolBoxTool(tool);
        connect(tool, SIGNAL(clicked()), this, SIGNAL(showAddWidgets()));
        
        tool = new Plasma::PushButton(i18n("Zoom In"));
        connect(tool, SIGNAL(clicked()), this, SIGNAL(zoomIn()));
        tool->resize(tool->sizeHint());
        addToolBoxTool(tool);
        
        tool = new Plasma::PushButton(i18n("Zoom Out"));
        connect(tool, SIGNAL(clicked()), this, SIGNAL(zoomOut()));
        tool->resize(tool->sizeHint());
        addToolBoxTool(tool);
    }
}

void Containment::loadConstraints(KConfigGroup* group)
{
/*    kDebug() << "!!!!!!!!!!!!initConstraints" << group->name() << type();
    kDebug() << "    location:" << group->readEntry("location", (int)d->location);
    kDebug() << "    geom:" << group->readEntry("geometry", geometry());
    kDebug() << "    formfactor:" << group->readEntry("formfactor", (int)d->formFactor);
    kDebug() << "    screen:" << group->readEntry("screen", d->screen);*/
    setGeometry(group->readEntry("geometry", geometry()));
    setLocation((Plasma::Location)group->readEntry("location", (int)d->location));
    setFormFactor((Plasma::FormFactor)group->readEntry("formfactor", (int)d->formFactor));
    setScreen(group->readEntry("screen", d->screen));
}

void Containment::saveConstraints(KConfigGroup* group) const
{
    group->writeEntry("screen", d->screen);
    group->writeEntry("formfactor", (int)d->formFactor);
    group->writeEntry("location", (int)d->location);
}

void Containment::containmentConstraintsUpdated(Plasma::Constraints constraints)
{
    if (d->toolbox && constraints & Plasma::ScreenConstraint) {
        d->toolbox->setPos(geometry().width() - d->toolbox->boundingRect().width(), 0);
    }
}

Containment::Type Containment::containmentType() const
{
    return DesktopContainment;
}

Corona* Containment::corona() const
{
    return dynamic_cast<Corona*>(scene());
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    //kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        QGraphicsItem::contextMenuEvent(event);
        return;
    }

    QPointF point = event->scenePos();
    QGraphicsItem* item = scene()->itemAt(point);
    if (item == this) {
        item = 0;
    }

    Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet) {
            break;
        }

        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << (QObject*)applet;
    if (!applet) {
        if (!scene() || static_cast<Corona*>(scene())->isImmutable()) {
            //kDebug() << "immutability";
            QGraphicsItem::contextMenuEvent(event);
            return;
        }

        //FIXME: change this to show this only in debug mode (or not at all?)
        //       before final release
        QList<QAction*> actions = contextActions();

        if (actions.count() < 1) {
            //kDebug() << "no applet, but no actions";
            QGraphicsItem::contextMenuEvent(event);
            return;
        }

        foreach(QAction* action, actions) {
            desktopMenu.addAction(action);
        }
    } else {
        bool hasEntries = false;   
        if (applet->hasConfigurationInterface()) {   
            QAction* configureApplet = new QAction(i18n("%1 Settings...", applet->name()), &desktopMenu);   
            connect(configureApplet, SIGNAL(triggered(bool)),   
                    applet, SLOT(showConfigurationInterface()));   
            desktopMenu.addAction(configureApplet);   
            hasEntries = true;   
        }   
    
        if (scene() && !static_cast<Corona*>(scene())->isImmutable()) {   
            QAction* closeApplet = new QAction(i18n("Remove this %1", applet->name()), &desktopMenu);   
            QVariant appletV;   
            appletV.setValue((QObject*)applet);   
            closeApplet->setData(appletV);   
            connect(closeApplet, SIGNAL(triggered(bool)),   
                    this, SLOT(destroyApplet()));   
            desktopMenu.addAction(closeApplet);   
            hasEntries = true;   
        }   
    
        QList<QAction*> actions = applet->contextActions();   
        if (!actions.isEmpty()) {   
            desktopMenu.addSeparator();   
            foreach(QAction* action, actions) {   
                desktopMenu.addAction(action);   
            }   
            hasEntries = true;   
        }   
    
        if (!hasEntries) {   
            QGraphicsItem::contextMenuEvent(event);   
            kDebug() << "no entries";   
            return;   
        }
    }

    event->accept();
    //kDebug() << "executing at" << event->screenPos();
    desktopMenu.exec(event->screenPos());
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor && layout()) {
        return;
    }

    //kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;
    Layout *lay = layout();
    setLayout(0);
    delete lay;
    lay = 0;

    switch (d->formFactor) {
        case Planar:
            lay = new FreeLayout(this);
            break;
        case Horizontal:
            lay = new BoxLayout(BoxLayout::LeftToRight, this);
            lay->setMargin(0);
            lay->setSpacing(0);
            break;
        case Vertical:
            lay = new BoxLayout(BoxLayout::TopToBottom, this);
            lay->setMargin(0);
            lay->setSpacing(0);
            break;
        case MediaCenter:
            //FIXME: need a layout type here!
            break;
        default:
            kDebug() << "This can't be happening! Or... can it? ;)";
            break;
    }

    if (lay) {
        foreach (Applet* applet, d->applets) {
            lay->addItem(applet);
            applet->updateConstraints(Plasma::FormFactorConstraint);
        }
    }

    updateConstraints(Plasma::FormFactorConstraint);
}

FormFactor Containment::formFactor() const
{
    return d->formFactor;
}

void Containment::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints(Plasma::LocationConstraint);
    }

    setScreen(screen());
    updateConstraints(Plasma::LocationConstraint);
}

Location Containment::location() const
{
    return d->location;
}

void Containment::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Containment::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& appletGeometry, bool delayInit)
{
    Applet* applet = Applet::loadApplet(name, id, args);
    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet;
    }

    addChild(applet);
    //panels don't want backgrounds, which is important when setting geometry
    if (type() == PanelContainment) {
        applet->setDrawStandardBackground(false);
    }

    //the applet needs to be given constraints before it can set its geometry
    applet->updateConstraints(Plasma::AllConstraints);

     //kDebug() << "adding applet" << applet->name() << "with a default geometry of" << appletGeometry << appletGeometry.isValid();
    if (appletGeometry.isValid()) {
        applet->setGeometry(appletGeometry);
    } else if (appletGeometry.x() != -1 && appletGeometry.y() != -1) {
        // yes, this means we can't have items start -1, -1
        applet->setGeometry(QRectF(appletGeometry.topLeft(),
                                   applet->sizeHint()));
    } else if (geometry().isValid()) {
        //TODO: Make sure new applets don't overlap with existing ones
        // Center exactly:
        QSizeF size = applet->sizeHint();
        qreal appletWidth = size.width();
        qreal appletHeight = size.height();
        qreal width = geometry().width();
        qreal height = geometry().height();
        //kDebug() << "measuring geometry with" << appletWidth << appletHeight << width << height;
        applet->setGeometry(QRectF(QPointF((width / 2) - (appletWidth / 2), (height / 2) - (appletHeight / 2)), size));
    }

    kDebug() << applet->name() << "sizehint:" << applet->sizeHint()
                               << "geometry:" << applet->geometry();

    if (!delayInit) {
        applet->init();
    }

    d->applets << applet;
    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));
    Phase::self()->animateItem(applet, Phase::Appear);

    emit appletAdded(applet);
    return applet;
}

void Containment::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    int index = d->applets.indexOf(applet);

    if (index > -1) {
        d->applets.removeAt(index);
    }
}

void Containment::appletAnimationComplete(QGraphicsItem *item, Plasma::Phase::Animation anim)
{
    if (anim == Phase::Disappear) {
        if (item->parentItem() == this) {
            Applet *applet = qgraphicsitem_cast<Applet*>(item);

            if (applet) {
                applet->destroy();
            }
        }
    } else if (anim == Phase::Appear) {
        if (type() == DesktopContainment) {
            item->installSceneEventFilter(this);
        }
    }
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int screen)
{
    // screen of -1 means no associated screen.
    // sanity check to make sure someone else doesn't have this screen already!
    if (type() == DesktopContainment && corona()) {
        Containment* currently = corona()->containmentForScreen(screen);
        if (currently && currently != this) {
            //kDebug() << "currently is on screen" << currently->screen() << "and is" << currently->name() << (QObject*)currently << (QObject*)this;
            currently->setScreen(-1);
        }
    }

    //kDebug() << "setting screen to" << screen;
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    if (screen < -1 || screen > numScreens - 1) {
        screen = -1;
    }

    //kDebug() << "setting screen to " << screen << "and type is" << type();
    if (screen > -1) {
        QRect r = desktop->screenGeometry(screen);

        if (type() == DesktopContainment) {
            // we need to find how many screens are to our top and left
            // to calculate the proper offsets for the margins.
            int x = r.x();
            int y = r.y();
            int screensLeft = 0;
            int screensAbove = 0;
            for (int i = 0; i < numScreens; ++i) {
                QRect otherScreen = desktop->screenGeometry(screen);
                if (x > otherScreen.x()) {
                    ++screensLeft;
                }

                if (y > otherScreen.y()) {
                    ++screensAbove;
                }
            }

            r.moveLeft(r.x() + INTER_CONTAINMENT_MARGIN * screensLeft);
            r.moveTop(r.y() + INTER_CONTAINMENT_MARGIN * screensAbove);

            // FIXME: positioning at this x,y will break if we switch between containments for a
            //        given screen! we should change the pos() on new containment setup.
            setGeometry(r);
            //kDebug() << "setting geometry to" << desktop->screenGeometry(screen) << r << geometry();
        } else if (type() == PanelContainment) {
            QRect r = desktop->screenGeometry(screen);
            //kDebug() << "we are a panel on" << r << ", let's move ourselves to a negative coordinate system" << -r.height() - INTER_CONTAINMENT_MARGIN;
            int vertOffset = r.height() + INTER_CONTAINMENT_MARGIN;
            translate(0, -vertOffset);
        }
    }

    d->screen = screen;
    updateConstraints(Plasma::ScreenConstraint);
}

int Containment::screen() const
{
    return d->screen;
}

KPluginInfo::List Containment::knownContainments(const QString &category,
                                                 const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    //kDebug() << "constraint was" << constraint << "which got us" << offers.count() << "matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Containment::knownContainmentsForMimetype(const QString &mimetype)
{
    QString constraint = QString("'%1' in MimeTypes").arg(mimetype);
    //kDebug() << "knownContainmentsForMimetype with" << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    return KPluginInfo::fromServices(offers);
}

void Containment::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    //kDebug() << "drop event:" << event->mimeData()->text();

    QString mimetype(static_cast<Corona*>(scene())->appletMimeType());

    if (event->mimeData()->hasFormat(mimetype) && scene()) {
        QString plasmoidName;
        plasmoidName = event->mimeData()->data(mimetype);
        QRectF geom(mapFromScene(event->scenePos()), QSize(0, 0));
        addApplet(plasmoidName, QVariantList(), 0, geom);
        event->acceptProposedAction();
    } else if (KUrl::List::canDecode(event->mimeData())) {
        KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
        foreach (const KUrl& url, urls) {
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QRectF geom(event->scenePos(), QSize(0, 0));
            QVariantList args;
            args << url.url();
            //             kDebug() << mimeName;
            KPluginInfo::List appletList = Applet::knownAppletsForMimetype(mimeName);

            if (appletList.isEmpty()) {
                // no special applet associated with this mimetype, let's
                addApplet("url", args, 0, geom);
            } else {
                //TODO: should we show a dialog here to choose which plasmoid load if
                //appletList.count() > 0?
                addApplet(appletList.first().pluginName(), args, 0, geom);
            }
        }
        event->acceptProposedAction();
    }
}

void Containment::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
    if (event->spontaneous()) {
        Applet::hoverEnterEvent(event);
    }
    Q_UNUSED(event)
}

void Containment::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //FIXME Qt4.4 check to see if this is still necessary to avoid unecessary repaints
    //            check with QT_FLUSH_PAINT=1 and mouse through applets that accept hover,
    //            applets that don't and system windows
//    Applet::hoverLeaveEvent(event);
    Q_UNUSED(event)
}

bool Containment::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Applet *applet = qgraphicsitem_cast<Applet*>(watched);
    //QEvent::GraphicsSceneHoverEnter

    // Otherwise we're watching something we shouldn't be...
    Q_ASSERT(applet!=0);
    if (!d->applets.contains(applet)) {
        return false;
    }

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
        if (!d->immutable && !applet->isImmutable() && !d->handles.contains(applet)) {
            AppletHandle *handle = new AppletHandle(this, applet);
            d->handles[applet] = handle;
            connect(handle, SIGNAL(disappearDone(AppletHandle*)),
                    this, SLOT(handleDisappeared(AppletHandle*)));
        }
        break;
    default:
        break;
    }

    return false;
}

void Containment::handleDisappeared(AppletHandle *handle)
{
    d->handles.remove(handle->applet());
    handle->deleteLater();
}

void Containment::emitLaunchActivated()
{
    kDebug();
    emit launchActivated();
}

void Containment::addToolBoxTool(QGraphicsItem *tool)
{
    if (!d->toolbox) {
        d->toolbox = new DesktopToolbox(this);
        d->toolbox->setPos(geometry().width() - d->toolbox->boundingRect().width(), 0);
    }

    d->toolbox->addTool(tool);
}

} // Plasma namespace

#include "containment.moc"

