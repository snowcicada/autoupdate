#include <QApplication>
#include "qtsingleapplication.h"
#include <QTextCodec>
#include "cautoupdate.h"
#include "ccurl.h"

int main(int argc, char *argv[])
{
    CCurl::GlobalInit();

//    QApplication a(argc, argv);
    QtSingleApplication a(argc, argv);
    if (a.IsRunning()) {
        return 0;
    }

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

    //设置软件的语言为:中文
    QTranslator translator;
    translator.load(a.applicationDirPath() + "/qt_zh_CN.qm");
    a.installTranslator(&translator);

    CAutoUpdate w;

    //自动更新
    if (true) {
        w.show();

        CCurl::GlobalCleanup();

        return a.exec();
    }

    //生成project.manifest
    return 1;
}
