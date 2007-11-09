#include "label.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QStyleOptionGraphicsItem>

namespace Plasma {

class Label::Private
{
    public:
        Private()
            : maximumWidth(9999)
        {}

        QString text;
        Qt::Alignment alignment;
        QPen textPen;
        QFont textFont;
        int maximumWidth;
};

Label::Label(Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    setAlignment(Qt::AlignHCenter);
    setPen(QPen(Qt::black, 1));
}

Label::~Label()
{
    delete d;
}

Qt::Orientations Label::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

bool Label::hasHeightForWidth() const
{
    return true;
}

qreal Label::heightForWidth(qreal w) const
{
    //FIXME: this looks a bit odd?
    QFontMetricsF m(d->textFont);
    //return m.boundingRect(QRectF(0, 0, w, 9999), d->alignment | Qt::TextWordWrap, d->text).height();
	return 0;
}

QSizeF Label::sizeHint() const
{
    QFontMetricsF m(d->textFont);
    return m.boundingRect(QRectF(0,0,9999,9999), d->alignment | Qt::TextWordWrap, d->text).size();
	//return m.boundingRect(QRectF(0, 0, d->maximumWidth, 9999), d->alignment | Qt::TextWordWrap, d->text).size();
}

void Label::setText(const QString& text)
{
    d->text = text;
    updateGeometry();
}

QString Label::text() const
{
    return d->text;
}

void Label::setAlignment(Qt::Alignment align)
{
    d->alignment = align;
}

Qt::Alignment Label::alignment() const
{
    return d->alignment;
}

void Label::setPen(const QPen& pen)
{
    d->textPen = pen;
    updateGeometry();
}

QPen Label::pen() const
{
    return d->textPen;
}

void Label::setMaximumWidth(int width)
{
    d->maximumWidth = width;
}

int Label::maximumWidth() const
{
    return d->maximumWidth;
}

void Label::setFont(const QFont& font)
{
    d->textFont = font;
    updateGeometry();
}

QFont Label::font() const
{
    return d->textFont;
}

void Label::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setPen(d->textPen);
    p->setFont(d->textFont);
    p->drawText(option->rect, d->alignment | Qt::TextWordWrap, d->text);
}

}
