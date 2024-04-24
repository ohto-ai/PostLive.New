#include "PostLiveClient.h"
#include "CommonConfigDialog.h"
#include "ffmpeg/FFmpegInputDevice.h"
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

    FFmpegInputDevice::updateInputDeviceList();
    auto deviceList = FFmpegInputDevice::filterVideoDevices();

    for (auto device : deviceList) {
        ui->menu_Devices->addAction(device->device_description.c_str(), [this, device] {
            ui->ffmpegWidget->stop();
            ui->ffmpegWidget->setInputDevice(device);
            ui->ffmpegWidget->play();
            });
    }
    m_gdiDevices.push_back(FFmpegInputDevice::make_gdi_device());
    m_gdiDevices.push_back(FFmpegInputDevice::make_gdi_device(windowTitle().toStdString()));
    for (auto& device : m_gdiDevices) {
        ui->menu_Devices->addAction(device.device_description.c_str(), [this, &device] {
            ui->ffmpegWidget->stop();
            ui->ffmpegWidget->setInputDevice(&device);
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

void PostLiveClient::closeEvent(QCloseEvent* event) {
    ui->ffmpegWidget->stop();
    QMainWindow::closeEvent(event);
}
