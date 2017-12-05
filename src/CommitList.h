#pragma once

#include <QAbstractScrollArea>

class CommitList : public QAbstractScrollArea
{
    Q_OBJECT
public:
    CommitList(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};
