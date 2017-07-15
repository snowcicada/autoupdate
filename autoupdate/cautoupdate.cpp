#include "cautoupdate.h"
#include "ui_cautoupdate.h"
#include "jsoncpp/CJson.h"
#include <QCryptographicHash>

#define SET_UPDATE  "config.ini"
#define APP_TITLE   "自动更新"
#define APP_NAME    "autoupdate.exe"

CAutoUpdate::CAutoUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAutoUpdate)
{
    ui->setupUi(this);

    initUi();

    readSettings();

    createLocalManifest(m_settings.strUpdateDir);
//    createRemoteManifest(m_settings.strUpdateDir);
}

CAutoUpdate::~CAutoUpdate()
{
    delete ui;
}

void CAutoUpdate::initUi()
{
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    setFixedSize(width(), height());
    setWindowTitle(APP_TITLE);
}

void CAutoUpdate::readSettings()
{
    QString strKey;
    QString strPath = qApp->applicationDirPath() + "/" + tr(SET_UPDATE);
    QSettings set(strPath, QSettings::IniFormat);

    strKey = "update/update_dir";
    if (set.contains(strKey)) {
        m_settings.strUpdateDir = set.value(strKey).toString();
    } else {
        m_settings.strUpdateDir = "../";
        set.setValue(strKey, m_settings.strUpdateDir);
    }

    strKey = "update/update_url";
    if (set.contains(strKey)) {
        m_settings.strUpdateUrl = set.value(strKey).toString();
    } else {
        set.setValue(strKey, "http://www.abc.com");
        QMessageBox::warning(this, "提示", "请配置更新地址！");
        return;
    }
}

bool CAutoUpdate::createLocalManifest(const QString &strPath)
{
    m_fileInfoList.clear();
    m_mapLocalManifest.clear();

    searchFile(m_fileInfoList, strPath);

    QString strFileName, strMd5;
    foreach (QFileInfo info, m_fileInfoList) {
        strFileName = info.filePath().replace(m_settings.strUpdateDir, "");
        QFile file(info.filePath());
        if (file.open(QFile::ReadOnly)) {
            strMd5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        } else {
            qDebug() << "读取文件失败!";
            m_mapLocalManifest.clear();
            return false;
        }
        file.close();
        m_mapLocalManifest[strFileName] = strMd5;
        qDebug() << strFileName << " -> " << strMd5;
    }

    return true;
}

bool CAutoUpdate::createRemoteManifest(const QString &strPath)
{
    m_fileInfoList.clear();
    m_mapLocalManifest.clear();

    searchFile(m_fileInfoList, strPath);

    Json::Value jsonVal;
    QString strFileName, strMd5;
    foreach (QFileInfo info, m_fileInfoList) {
        strFileName = info.filePath().replace(m_settings.strUpdateDir, "");
        QFile file(info.filePath());
        if (file.open(QFile::ReadOnly)) {
            strMd5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        } else {
            qDebug() << "读取文件失败!";
            m_mapLocalManifest.clear();
            return false;
        }
        file.close();
        jsonVal[strFileName.toStdString()] = strMd5.toStdString();
    }

    QFile file("project.manifest");
    if (file.open(QFile::WriteOnly)) {
        file.write(jsonVal.toStyledString().c_str());
    }
    file.close();

    return true;
}

void CAutoUpdate::searchFile(QFileInfoList &infoList, const QString &strPath)
{
    QString strCurrentPath = getCurrentDirName();
    QDir dir(strPath);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo info, fileList) {
        if (info.isFile()) {
            infoList.append(info);
        } else if (info.isDir() && info.fileName() != "." && info.fileName() != ".." && strCurrentPath != info.fileName()) {
            searchFile(infoList, info.filePath());
        }
    }
}

QString CAutoUpdate::getCurrentDirName()
{
    QString strPath = QDir::currentPath();
    QStringList strList = strPath.split('/');
    if (!strList.isEmpty()) {
        return strList.last();
    }
    return "";
}

