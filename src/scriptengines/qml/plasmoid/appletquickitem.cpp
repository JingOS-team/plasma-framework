/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
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

#include "appletquickitem.h"

#include <QQmlComponent>
#include <QQmlExpression>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQmlContext>

#include <QDebug>

#include <klocalizedstring.h>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <kdeclarative/qmlobject.h>
#include <plasma/scripting/appletscript.h>

#include <packageurlinterceptor.h>

QHash<QObject *, AppletQuickItem *> AppletQuickItemPrivate::s_rootObjects = QHash<QObject *, AppletQuickItem *>();

AppletQuickItemPrivate::AppletQuickItemPrivate(Plasma::Applet *a)
    : switchWidth(-1),
      switchHeight(-1),
      applet(a),
      expanded(false)
{
}



AppletQuickItem::AppletQuickItem(Plasma::Applet *applet, QQuickItem *parent)
    : QQuickItem(parent),
      d(new AppletQuickItemPrivate(applet))
{
    if (d->applet) {
        d->appletPackage = d->applet->package();
    }
    if (d->applet && d->applet->containment() && d->applet->containment()->corona()) {
        d->coronaPackage = d->applet->containment()->corona()->package();
    }

    d->compactRepresentationCheckTimer.setSingleShot(true);
    d->compactRepresentationCheckTimer.setInterval(250);
    connect (&d->compactRepresentationCheckTimer, SIGNAL(timeout()),
             this, SLOT(compactRepresentationCheck()));
    d->compactRepresentationCheckTimer.start();

    d->fullRepresentationResizeTimer.setSingleShot(true);
    d->fullRepresentationResizeTimer.setInterval(250);
    connect (&d->fullRepresentationResizeTimer, &QTimer::timeout,
             [=]() {
                KConfigGroup cg = d->applet->config();
                cg = KConfigGroup(&cg, "PopupApplet");
                cg.writeEntry("DialogWidth", d->fullRepresentationItem.data()->property("width").toInt());
                cg.writeEntry("DialogHeight", d->fullRepresentationItem.data()->property("height").toInt());
            }
    );



    d->qmlObject = new KDeclarative::QmlObject(this);
    d->qmlObject->setInitializationDelayed(true);

    // set the graphicObject dynamic property on applet
    d->applet->setProperty("_plasma_graphicObject", QVariant::fromValue(this));
    setProperty("_plasma_applet", QVariant::fromValue(applet));
}

AppletQuickItem::~AppletQuickItem()
{
    //Here the order is important
    delete d->compactRepresentationItem.data();
    delete d->fullRepresentationItem.data();
    delete d->compactRepresentationExpanderItem.data();

    AppletQuickItemPrivate::s_rootObjects.remove(d->qmlObject->engine());
}

Plasma::Applet *AppletQuickItem::applet() const
{
    return d->applet;
}

void AppletQuickItem::init()
{
    if (AppletQuickItemPrivate::s_rootObjects.contains(this)) {
        return;
    }

    AppletQuickItemPrivate::s_rootObjects[d->qmlObject->engine()] = this;

    Q_ASSERT(d->applet);

    //Initialize the main QML file
    QQmlEngine *engine = d->qmlObject->engine();

    PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine, d->applet->package());
    interceptor->addAllowedPath(d->coronaPackage.path());
    engine->setUrlInterceptor(interceptor);

    d->qmlObject->setSource(QUrl::fromLocalFile(d->applet->package().filePath("mainscript")));

    if (!engine || !engine->rootContext() || !engine->rootContext()->isValid() || d->qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, d->qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
        }
        reason = i18n("Error loading QML file: %1", reason);

        d->qmlObject->setSource(QUrl::fromLocalFile(d->coronaPackage.filePath("appleterror")));
        d->qmlObject->completeInitialization();


        //even the error message QML may fail
        if (d->qmlObject->mainComponent()->isError()) {
            return;
        } else {
            d->qmlObject->rootObject()->setProperty("reason", reason);
        }

        d->applet->setLaunchErrorMessage(reason);
    }

    engine->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    QVariantHash initialProperties;
    initialProperties["width"] = width();
    initialProperties["height"] = height();
    d->qmlObject->completeInitialization(initialProperties);

    //default fullrepresentation is our root main component, if none specified
    if (!d->fullRepresentation) {
        d->fullRepresentation = d->qmlObject->mainComponent();
        d->fullRepresentationItem = d->qmlObject->rootObject();
        emit fullRepresentationChanged(d->fullRepresentation.data());
    }

    //default d->compactRepresentation is a simple icon provided by the shell package
    if (!d->compactRepresentation) {
        d->compactRepresentation = new QQmlComponent(engine, this);
        d->compactRepresentation.data()->loadUrl(QUrl::fromLocalFile(d->coronaPackage.filePath("defaultcompactrepresentation")));
        emit compactRepresentationChanged(d->compactRepresentation.data());
    }

    //default d->compactRepresentationExpander is the popup in which fullRepresentation goes
    if (!d->compactRepresentationExpander) {
        d->compactRepresentationExpander = new QQmlComponent(engine, this);
        d->compactRepresentationExpander.data()->loadUrl(QUrl::fromLocalFile(d->coronaPackage.filePath("compactapplet")));
        emit compactRepresentationExpanderItemChanged(d->compactRepresentationExpander.data());
    }

}

Plasma::Package AppletQuickItem::appletPackage() const
{
    return d->appletPackage;
}

void AppletQuickItem::setAppletPackage(const Plasma::Package &package)
{
    d->appletPackage = package;
}

Plasma::Package AppletQuickItem::coronaPackage() const
{
    return d->coronaPackage;
}

void AppletQuickItem::setCoronaPackage(const Plasma::Package &package)
{
    d->coronaPackage = package;
}

int AppletQuickItem::switchWidth() const
{
    return d->switchWidth;
}

void AppletQuickItem::setSwitchWidth(int width)
{
    if (d->switchWidth == width) {
        return;
    }

    d->switchWidth = width;
    emit switchWidthChanged(width);
}

int AppletQuickItem::switchHeight() const
{
    return d->switchHeight;
}

void AppletQuickItem::setSwitchHeight(int width)
{
    if (d->switchHeight == width) {
        return;
    }

    d->switchHeight = width;
    emit switchHeightChanged(width);
}

QQmlComponent *AppletQuickItem::compactRepresentation()
{
    return d->compactRepresentation.data();
}

void AppletQuickItem::setCompactRepresentation(QQmlComponent *component)
{
    if (d->compactRepresentation.data() == component) {
        return;
    }

    d->compactRepresentation = component;
    emit compactRepresentationChanged(component);
}


QQmlComponent *AppletQuickItem::fullRepresentation()
{
    return d->fullRepresentation.data();
}

void AppletQuickItem::setFullRepresentation(QQmlComponent *component)
{
    if (d->fullRepresentation.data() == component) {
        return;
    }

    d->fullRepresentation = component;
    emit fullRepresentationChanged(component);
}

QQmlComponent *AppletQuickItem::preferredRepresentation()
{
    return d->preferredRepresentation.data();
}

void AppletQuickItem::setPreferredRepresentation(QQmlComponent *component)
{
    if (d->preferredRepresentation.data() == component) {
        return;
    }

    d->preferredRepresentation = component;
    emit preferredRepresentationChanged(component);
}

bool AppletQuickItem::isExpanded() const
{
    return d->expanded;
}

void AppletQuickItem::setExpanded(bool expanded)
{
    if (d->applet->isContainment()) {
        expanded = true;
    }

    //if there is no compact representation it means it's always expanded
    //Containnments are always expanded
    if (d->expanded == expanded) {
        return;
    }

    createFullRepresentationItem();
    createCompactRepresentationExpanderItem();

    d->expanded = expanded;
    emit expandedChanged(expanded);
}

////////////Internals


KDeclarative::QmlObject *AppletQuickItem::qmlObject()
{
    return d->qmlObject;
}

QObject *AppletQuickItem::compactRepresentationItem()
{
    return d->compactRepresentationItem.data();
}

QObject *AppletQuickItem::fullRepresentationItem()
{
    return d->fullRepresentationItem.data();
}

QObject *AppletQuickItem::compactRepresentationExpanderItem()
{
    return d->compactRepresentationExpanderItem.data();
}



QObject *AppletQuickItem::createCompactRepresentationItem()
{
    if (!d->compactRepresentation) {
        return 0;
    }

    if (d->compactRepresentationItem) {
        return d->compactRepresentationItem.data();
    }

    d->compactRepresentationItem = d->qmlObject->createObjectFromComponent(d->compactRepresentation.data(), QtQml::qmlContext(d->qmlObject->rootObject()));

    emit compactRepresentationItemChanged(d->compactRepresentationItem.data());

    return d->compactRepresentationItem.data();
}

QObject *AppletQuickItem::createFullRepresentationItem()
{
    if (d->fullRepresentationItem) {
        return d->fullRepresentationItem.data();
    }

    if (d->fullRepresentation) {
        d->fullRepresentationItem = d->qmlObject->createObjectFromComponent(d->fullRepresentation.data(), QtQml::qmlContext(d->qmlObject->rootObject()));
    } else {
        d->fullRepresentation = d->qmlObject->mainComponent();
        d->fullRepresentationItem = d->qmlObject->rootObject();
        emit fullRepresentationChanged(d->fullRepresentation.data());
    }

    QQuickItem *graphicsObj = qobject_cast<QQuickItem *>(d->fullRepresentationItem.data());
    connect (graphicsObj, &QQuickItem::widthChanged, [=]() {
        d->fullRepresentationResizeTimer.start();
    });
    connect (graphicsObj, &QQuickItem::heightChanged, [=]() {
        d->fullRepresentationResizeTimer.start();
    });

    emit fullRepresentationItemChanged(d->fullRepresentationItem.data());

    return d->fullRepresentationItem.data();
}

QObject *AppletQuickItem::createCompactRepresentationExpanderItem()
{
    if (!d->compactRepresentationExpander) {
        return 0;
    }

    if (d->compactRepresentationExpanderItem) {
        return d->compactRepresentationExpanderItem.data();
    }

    d->compactRepresentationExpanderItem = d->qmlObject->createObjectFromComponent(d->compactRepresentationExpander.data(), QtQml::qmlContext(d->qmlObject->rootObject()));


    d->compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant::fromValue(createCompactRepresentationItem()));
    d->compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant::fromValue(createFullRepresentationItem()));

    emit compactRepresentationExpanderItemChanged(d->compactRepresentationExpanderItem.data());

    return d->compactRepresentationExpanderItem.data();
}

void AppletQuickItem::connectLayoutAttached(QObject *item)
{
    QObject *layout = 0;

    //Extract the representation's Layout, if any
    //No Item?
    if (!item) {
        return;
    }

    //Search a child that has the needed Layout properties
    //HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    foreach (QObject *child, item->children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
            child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
            child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
            child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
        ) {
            layout = child;
        }
    }

    if (!layout) {
        return;
    }

    //propagate all the size hints
    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");

    //HACK: check the Layout properties we wrote
    QQmlProperty p(this, "Layout.minimumWidth", QtQml::qmlContext(d->qmlObject->rootObject()));

    QObject *ownLayout = 0;

    foreach (QObject *child, children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
            child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
            child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
            child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
        ) {
            ownLayout = child;
        }
    }

    //this should never happen, since we ask to create it if doesn't exists
    if (!ownLayout) {
        return;
    }

    //if the representation didn't change, don't do anything
    if (d->representationLayout.data() == layout) {
        return;
    }

    if (d->representationLayout) {
        disconnect(d->representationLayout.data(), 0, this, 0);
    }

    //Here we can't use the new connect syntax because we can't link against QtQuick layouts
    connect(layout, SIGNAL(minimumWidthChanged()),
            this, SLOT(minimumWidthChanged()));
    connect(layout, SIGNAL(minimumHeightChanged()),
            this, SLOT(minimumHeightChanged()));

    connect(layout, SIGNAL(preferredWidthChanged()),
            this, SLOT(preferredWidthChanged()));
    connect(layout, SIGNAL(preferredHeightChanged()),
            this, SLOT(preferredHeightChanged()));

    connect(layout, SIGNAL(maximumWidthChanged()),
            this, SLOT(maximumWidthChanged()));
    connect(layout, SIGNAL(maximumHeightChanged()),
            this, SLOT(maximumHeightChanged()));

    connect(layout, SIGNAL(fillWidthChanged()),
            this, SLOT(fillWidthChanged()));
    connect(layout, SIGNAL(fillHeightChanged()),
            this, SLOT(fillHeightChanged()));

    d->representationLayout = layout;
    d->ownLayout = ownLayout;

    propagateSizeHint("minimumWidth");
    propagateSizeHint("minimumHeight");
    propagateSizeHint("preferredWidth");
    propagateSizeHint("preferredHeight");
    propagateSizeHint("maximumWidth");
    propagateSizeHint("maximumHeight");
    propagateSizeHint("fillWidth");
    propagateSizeHint("fillHeight");
}

void AppletQuickItem::propagateSizeHint(const QByteArray &layoutProperty)
{
    if (d->ownLayout && d->representationLayout) {
        d->ownLayout.data()->setProperty(layoutProperty, d->representationLayout.data()->property(layoutProperty));
    }
}

void AppletQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->compactRepresentationCheckTimer.start();
}

void AppletQuickItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the representations if needed
        if (value.window) {
            init();
        }
    }

    QQuickItem::itemChange(change, value);
}


//// Slots

void AppletQuickItem::compactRepresentationCheck()
{
    //ignore 0,0 sizes;
    if (width() <= 0 && height() <= 0) {
        return;
    }

    bool full = false;

    if (d->applet->isContainment()) {
        full = true;

    } else {
        if (d->switchWidth > 0 && d->switchHeight > 0) {
            full = width() > d->switchWidth && height() > d->switchHeight;
        //if a size to switch wasn't set, determine what representation to always chose
        } else {
            //preferred representation set?
            if (d->preferredRepresentation) {
                full = d->preferredRepresentation.data() == d->fullRepresentation.data();
            //Otherwise, base on FormFactor
            } else {
                full = (d->applet->formFactor() != Plasma::Types::Horizontal && d->applet->formFactor() != Plasma::Types::Vertical);
            }
        }

        if ((full && d->fullRepresentationItem && d->fullRepresentationItem.data() == d->currentRepresentationItem.data()) ||
            (!full && d->compactRepresentationItem && d->compactRepresentationItem.data() == d->currentRepresentationItem.data())
        ) {
            return;
        }
    }

    //Expanded
    if (full) {
        QQuickItem *item = qobject_cast<QQuickItem *>(createFullRepresentationItem());

        if (item) {
            item->setParentItem(this);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(d->qmlObject->rootObject()), item, "parent");
                QQmlProperty prop(item, "anchors.fill");
                prop.write(expr.evaluate());
            }
            if (d->compactRepresentationItem) {
                d->compactRepresentationItem.data()->setProperty("visible", false);
            }
            if (d->compactRepresentationExpanderItem) {
                d->compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant());
                d->compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant());
            }

            d->currentRepresentationItem = item;
            connectLayoutAttached(item);
        }

    //Icon
    } else {
        QQuickItem *compactItem = qobject_cast<QQuickItem *>(createCompactRepresentationItem());

        if (compactItem) {
            //set the root item as the main visible item
            compactItem->setParentItem(this);
            compactItem->setVisible(true);
            {
                //set anchors
                QQmlExpression expr(QtQml::qmlContext(d->qmlObject->rootObject()), compactItem, "parent");
                QQmlProperty prop(compactItem, "anchors.fill");
                prop.write(expr.evaluate());
            }

            if (d->fullRepresentationItem) {
                d->fullRepresentationItem.data()->setProperty("parent", QVariant());
            }

            if (d->compactRepresentationExpanderItem) {
                d->compactRepresentationExpanderItem.data()->setProperty("compactRepresentation", QVariant::fromValue(compactItem));
                d->compactRepresentationExpanderItem.data()->setProperty("fullRepresentation", QVariant::fromValue(createFullRepresentationItem()));
            }

            d->currentRepresentationItem = compactItem;
            connectLayoutAttached(compactItem);
        }
    }
}

void AppletQuickItem::minimumWidthChanged()
{
    propagateSizeHint("minimumWidth");
}

void AppletQuickItem::minimumHeightChanged()
{
    propagateSizeHint("minimumHeight");
}

void AppletQuickItem::preferredWidthChanged()
{
    propagateSizeHint("preferredWidth");
}

void AppletQuickItem::preferredHeightChanged()
{
    propagateSizeHint("preferredHeight");
}

void AppletQuickItem::maximumWidthChanged()
{
    propagateSizeHint("maximumWidth");
}

void AppletQuickItem::maximumHeightChanged()
{
    propagateSizeHint("maximumHeight");
}

void AppletQuickItem::fillWidthChanged()
{
    propagateSizeHint("fillWidth");
}

void AppletQuickItem::fillHeightChanged()
{
    propagateSizeHint("fillHeight");
}


#include "moc_appletquickitem.cpp"

