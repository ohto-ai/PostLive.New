#include "FileDropZone.h"
#include <QDragEnterEvent>
#include <QMimeData>

FileDropZone::FileDropZone(QWidget* parent)
    : QWidget(parent) {
    setAcceptDrops(true);
}

FileDropZone::~FileDropZone() {}

void FileDropZone::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QStringList paths;
        for (auto& url : urlList) {
            paths.push_back(url.toLocalFile());
        }
        emit onFileDropped(paths);
    }
}

void FileDropZone::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}
