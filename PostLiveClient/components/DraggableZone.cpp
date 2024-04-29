#include "DraggableZone.h"
#include <QMouseEvent>

DraggableZone::DraggableZone(QWidget* parent)
    : QWidget(parent) {}

DraggableZone::~DraggableZone() {}

void DraggableZone::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // find top widget
        topWidget = this;
        while (topWidget->parentWidget() != nullptr) {
            topWidget = topWidget->parentWidget();
        }
        m_dragPosition = event->globalPos() - topWidget->pos();
        event->accept();
    }
    else {
        QWidget::mousePressEvent(event);
    }
}

void DraggableZone::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!m_dragPosition.isNull()) {
            topWidget->move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }
    else {
        QWidget::mouseMoveEvent(event);
    }
}

void DraggableZone::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = QPoint();
        event->accept();
    }
    else {
        QWidget::mouseReleaseEvent(event);
    }
}
