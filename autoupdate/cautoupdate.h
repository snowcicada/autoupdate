#ifndef CAUTOUPDATE_H
#define CAUTOUPDATE_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include "ccurl.h"

namespace Ui {
class CAutoUpdate;
}

struct stSettings {
    QString strUpdateDir; //更新目录
    QString strUpdateUrl; //更新网址
    QStringList strDirWhiteList; //白名单目录
};

class CAutoUpdate : public QDialog
{
    Q_OBJECT
    
public:
    explicit CAutoUpdate(QWidget *parent = 0);
    ~CAutoUpdate();

private:
    void initUi();
    void readSettings();
    bool createLocalManifest(const QString& strPath);
    bool createRemoteManifest(const QString& strPath);
    void searchFile(QFileInfoList &infoList, const QString& strPath);
    QString getCurrentDirName();

private slots:
    void slotTimeout();
    
private:
    Ui::CAutoUpdate *ui;

    stSettings m_settings;
    QFileInfoList m_fileInfoList;
    std::map<QString, QString> m_mapLocalManifest;
    CCurl m_curl;
};
#endif // CAUTOUPDATE_H
