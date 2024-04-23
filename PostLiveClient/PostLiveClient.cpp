#include "PostLiveClient.h"
#include "CommonConfigDialog.h"
#include <QPushButton>
#include <QStyle>
#include <QMouseEvent>

extern "C" {
    #include <libavdevice/avdevice.h>
}
#include <QFileDialog>
#include <QDebug>

PostLiveClient::PostLiveClient(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::PostLiveClientClass()) {
    ui->setupUi(this);

    connect(ui->actionFile, &QAction::triggered, [this] {
        auto url = QFileDialog::getOpenFileUrl(this, "Open File", QUrl(), "All Files (*)");
        if (url.isEmpty()) {
            return;
        }
        ui->ffmpegWidget->stop();
        ui->ffmpegWidget->setUrl(url.toLocalFile());
        ui->ffmpegWidget->play();
        });

    auto deviceList = FFmpegVideo::updateDeviceList();

    for (int i = 0; i < deviceList->nb_devices; i++) {
        auto url = QString::asprintf("video=%s", deviceList->devices[i]->device_name);
        ui->menu_Devices->addAction(deviceList->devices[i]->device_description, [this, url] {
            ui->ffmpegWidget->stop();
            ui->ffmpegWidget->setUrl(url);
            ui->ffmpegWidget->play();
            });
    }

    connect(ui->action_Server, &QAction::triggered, [this] {

        CommonConfigItemList().addBool("key_bool", "Enable", "", true)
            .addInt("key_int", "Port", "", 8080)
            .addDouble("key_double", "Double", "", 3.14)
            .addString("key_string", "String", "", "Hello")
            .addLabel("This is a label")
            .addEnum("key_enum", "Enum", "", QStringList() << "A" << "B" << "C", "A")
            .execDialog(this, "");
        }
    );
}

PostLiveClient::~PostLiveClient() {
    delete ui;
}


void PostLiveClient::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - this->pos();
        event->accept();
    }
    else {
        QMainWindow::mousePressEvent(event);
    }
}

void PostLiveClient::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!m_dragPosition.isNull()) {
            this->move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }
    else {
        QMainWindow::mouseMoveEvent(event);
    }
}

void PostLiveClient::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = QPoint();
        event->accept();
    }
    else {
        QMainWindow::mouseReleaseEvent(event);
    }
}
