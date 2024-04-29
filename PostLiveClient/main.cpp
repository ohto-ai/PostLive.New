#include "PostLiveClient.h"
#include "LoginPanel.h"

#if defined(Q_OS_WINDOWS)
#include "components/WinDbgHelper.h"
#endif

#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QMovie>
#include <QDesktopWidget>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

#if defined(Q_OS_WINDOWS)
    WinDbgHelper dbg;
#endif

    LoginPanel p;

    if (p.exec() == QDialog::Rejected) {
        return 0;
    }

    QSplashScreen splash;
    QMovie movie(":/main/res/splash.gif");

    QObject::connect(&movie, &QMovie::frameChanged, &splash, [&]() {
        splash.setPixmap(movie.currentPixmap());
        });
    movie.start();
    splash.show();
    splash.showMessage(QObject::tr("Loading..."), Qt::AlignCenter, Qt::white);

    PostLiveClient w;
    splash.finish(&w);
    w.show();
    return a.exec();
}
