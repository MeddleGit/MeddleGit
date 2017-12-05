#include "CommitList.h"

#include <QPaintEvent>
#include <QPainter>

CommitList::CommitList(QWidget* parent)
    : QAbstractScrollArea(parent)
{
}

void CommitList::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(viewport());
    p.drawText(10, 10, 100, 100, 0, "drawText");
}
