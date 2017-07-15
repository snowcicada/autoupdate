#include <QApplication>
#include <QTextCodec>
#include "cautoupdate.h"
#include "ccurl.h"

int main(int argc, char *argv[])
{
    CCurl::GlobalInit();

    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

    CAutoUpdate w;
    w.show();

    CCurl::GlobalCleanup();
    
    return a.exec();
}
