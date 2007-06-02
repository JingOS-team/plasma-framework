#ifndef APPLET_LAYOUT_H
#define APPLET_LAYOUT_H

#include <QLayout>
#include <QSize>

#include <plasma/plasma_export.h>

class QLayoutItem;
class QRect;
namespace Plasma
{

/**
 * AppletLayout is our custom layouting engine
 * for applets. It reacts to state transition (removal,addition
 * of applets) by emitting corresponding signals which AppletCompositor
 * can intercept and respond with some nice animation/effect.
 */
class PLASMA_EXPORT AppletLayout : public QLayout
{
        Q_OBJECT
    public:
        AppletLayout(QWidget *parent);
        ~AppletLayout();

        virtual void addItem(QLayoutItem *item);
        virtual Qt::Orientations expandingDirections() const;
        virtual bool hasHeightForWidth() const;
        virtual int heightForWidth(int) const;
        virtual int count() const;
        virtual QLayoutItem *itemAt(int index) const;
        virtual QSize minimumSize() const;
        virtual void setGeometry(const QRect &rect);
        virtual QSize sizeHint() const;
        virtual QLayoutItem *takeAt(int index);
    Q_SIGNALS:
        void mergeTwoApplets();
        void splitTwoApplets();
    protected:
        int layoutApplets(const QRect &rect, bool computeHeightOnly) const;
    private:
        class Private;
        Private *d;
};

}

#endif
