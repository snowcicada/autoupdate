#include <QApplication>
#include <QTextCodec>
#include "cautoupdate.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());

    CAutoUpdate w;
    w.show();
    
    return a.exec();
}
