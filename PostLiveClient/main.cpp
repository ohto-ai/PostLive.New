#include "PostLiveClient.h"
#include "LoginPanel.h"

#if defined(Q_OS_WINDOWS)
    #include "components/WinDbgHelper.h"
#endif

#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

#if defined(Q_OS_WINDOWS)
    WinDbgHelper dbg;
#endif

    LoginPanel p;

    if (p.exec() == QDialog::Rejected) {
        return 0;
    }

    PostLiveClient w;
    w.show();
    return a.exec();
}