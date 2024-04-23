#pragma once

#include <QLabel>
#include <QPixmap>

class ImageLabel  : public QLabel
{
    Q_OBJECT

    Q_PROPERTY(bool covered READ covered WRITE setCovered)

public:
    ImageLabel(QWidget *parent);
    ~ImageLabel();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    int margin() const;
    bool covered() const;
    bool scaledContents() const;
    Qt::Alignment alignment() const;
    QPixmap pixmap() const;
public slots:
    void setPixmap(const QPixmap&);
    void setAlignment(Qt::Alignment alignment);
    void setScaledContents(bool scaled);
    void setCovered(bool covered);
    void setMargin(int margin);

private:
    void updateScaledPixmap();

private:
    QPixmap m_pixmap;
    QPixmap m_scaledPixmap;
    QPoint m_pixmapPos;
    Qt::Alignment m_alignment;
    bool m_scaledContents = true;
    bool m_covered = false;
    int m_margin = 0;
};
