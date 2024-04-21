#include "PostLiveClient.h"

#if defined(Q_OS_WINDOWS)
    #include "WinDbgHelper.h"
#endif

#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

#if defined(Q_OS_WINDOWS)
    WinDbgHelper dbg;
#endif

    PostLiveClient w;
    w.show();
    return a.exec();
}