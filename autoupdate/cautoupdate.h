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
    QString strApp;   //app
    QStringList strNotUpdateDirList; //不更新的目录
    QStringList strNotUpdateFileList; //不更新的文件
    int nUpdateInterval; //更新间隔(秒)
};

typedef std::map<QString, QString> QQMAP;
typedef std::list<QString> FileList;

class CAutoUpdate : public QDialog
{
    Q_OBJECT
    
public:
    explicit CAutoUpdate(QWidget *parent = 0);
    ~CAutoUpdate();

protected:
    void closeEvent(QCloseEvent *e);

private:
    void initUi();
    void readSettings();
    bool createLocalManifest(QQMAP& mapManifest, const QString& strPath);
    bool createRemoteManifest(QQMAP& mapManifest, const QString& strPath);
    void searchFile(QFileInfoList &infoList, const QString& strPath);
    void searchFileEx(QFileInfoList &infoList, const QString& strPath);
    QString getCurrentDirName();
    bool getRemoteManifest(QQMAP& mapManifest);
    void compareLocalRemoteManifest(QQMAP& local, QQMAP& remote, FileList& fileList);
    bool downloadDiffFiles(const FileList& fileList);
    QString removeSetComment(const QString& str);
    bool canUpdate();
    void removeAllFiles(const QString& strPath);
    void copyAllFiles(const QString& strSrcPath, const QString& strDstPath);

private slots:
    void slotTimeout();
    void slotActQuit();
    void slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    
private:
    Ui::CAutoUpdate *ui;

    stSettings m_settings;
    QFileInfoList m_fileInfoList;
    QQMAP m_mapLocalManifest;
    QQMAP m_mapRemoteManifest;
    CCurl m_curl;
    QSystemTrayIcon* m_pSysTrayIcon;
    QMenu* m_pSysTrayMenu;
    QAction* m_pActQuit;
    QTimer m_timer;
};
#endif // CAUTOUPDATE_H
