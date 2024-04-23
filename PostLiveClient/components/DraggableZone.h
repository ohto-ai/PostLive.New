#pragma once

#include <QWidget>

class DraggableZone  : public QWidget
{
    Q_OBJECT

public:
    DraggableZone(QWidget *parent);
    ~DraggableZone();

public:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPoint m_dragPosition;
    QWidget* topWidget;
};
