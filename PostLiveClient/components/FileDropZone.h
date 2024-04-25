#pragma once

#include <QWidget>

class FileDropZone  : public QWidget
{
    Q_OBJECT

public:
    FileDropZone(QWidget *parent = nullptr);
    ~FileDropZone();

signals:
    void onFileDropped(const QStringList& paths);

public:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
};
