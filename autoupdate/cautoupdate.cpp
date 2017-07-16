#include "cautoupdate.h"
#include "ui_cautoupdate.h"
#include "jsoncpp/CJson.h"
#include <QCryptographicHash>

#define SET_UPDATE          "config.ini"
#define APP_TITLE           "自动更新"
#define APP_NAME            "autoupdate.exe"
#define PROJECT_MANIFEST    "project.manifest"

CAutoUpdate::CAutoUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAutoUpdate)
{
    ui->setupUi(this);

    initUi();
    readSettings();

//    createRemoteManifest(m_mapLocalManifest, m_settings.strUpdateDir);

    QTimer::singleShot(1000, this, SLOT(slotTimeout()));
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
    ui->labelFileName->clear();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(1);
}

void CAutoUpdate::readSettings()
{
    QString strKey, strTmp;
    QString strPath = qApp->applicationDirPath() + "/" + tr(SET_UPDATE);
    QSettings set(strPath, QSettings::IniFormat);

    strKey = "update/update_dir";
    if (set.contains(strKey)) {
        m_settings.strUpdateDir = set.value(strKey).toString();
    } else {
        m_settings.strUpdateDir = "../";
        set.setValue(strKey, m_settings.strUpdateDir);
    }

    strKey = "update/dir_whitelist";
    if (set.contains(strKey)) {
        strTmp = set.value(strKey).toString();
        m_settings.strDirWhiteList = strTmp.split(';');
    } else {
        set.setValue(strKey, "");
    }

    strKey = "update/update_url";
    if (set.contains(strKey)) {
        m_settings.strUpdateUrl = set.value(strKey).toString();
    } else {
        set.setValue(strKey, "http://www.abc.com");
        QMessageBox::warning(this, "提示", "请配置更新地址update_url！");
        return;
    }
}

bool CAutoUpdate::createLocalManifest(QQMAP &mapManifest, const QString &strPath)
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
        QCoreApplication::processEvents();
//        qDebug() << strFileName << " -> " << strMd5;
    }

    return true;
}

bool CAutoUpdate::createRemoteManifest(QQMAP &mapManifest, const QString &strPath)
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
        m_mapLocalManifest[strFileName] = strMd5;
        jsonVal[strFileName.toStdString()] = strMd5.toStdString();
        QCoreApplication::processEvents();
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
        } else if (info.isDir() && info.fileName() != "." &&
                   info.fileName() != ".." && strCurrentPath != info.fileName() &&
                   m_settings.strDirWhiteList.indexOf(info.fileName()) == -1) {
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

bool CAutoUpdate::getRemoteManifest(QQMAP &mapManifest)
{
    //获取远端信息
    QString strUrlManifest = m_settings.strUpdateUrl + "/" + tr(PROJECT_MANIFEST);
    QString strOutput;
    bool bRes = m_curl.Get(strUrlManifest, strOutput);
    if (!bRes) {
        return false;
    }

    JsonStringMap kvMap;
    CJson::JsonToMap(strOutput.toStdString(), kvMap);

    mapManifest.clear();
    for (auto& it : kvMap) {
        mapManifest[QString::fromStdString(it.first)] = QString::fromStdString(it.second);
    }
    return true;
}

void CAutoUpdate::compareLocalRemoteManifest(QQMAP &local, QQMAP &remote, FileList &fileList)
{
    for (auto& it : remote) {
        auto fit = local.find(it.first);
        if (local.end() == fit) {
            //新增文件
            fileList.push_back(it.first);
        } else if (fit->second != it.second) {
            //md5不同
            fileList.push_back(it.first);
        }
    }
}

bool CAutoUpdate::downloadDiffFiles(const FileList &fileList)
{
    static const QString s_tmpDir = "tmp";
    QString strUrl;

    //每次重新创建目录
    QDir dir = QDir::current();
    if (dir.exists(s_tmpDir)) {
        dir.remove(s_tmpDir);
    }
    dir.mkdir(s_tmpDir);

    int index = 0, count = 0;
    QString strFileName, strName, strPath;
    ui->progressBar->setMaximum(fileList.size());
    ui->progressBar->setValue(0);
    for (auto& it : fileList) {
        //创建目录
        strFileName = it;
        index = strFileName.lastIndexOf('/');
        if (-1 != index) {
            //带目录的文件
            strName = strFileName.right(strFileName.length() - index - 1);
            strPath = strFileName.left(index);
        } else {
            strName = strFileName;
            strPath = "";
        }
        if (!strPath.isEmpty()) {
            dir.mkpath(strPath);
        }
//        qDebug() << "name=" << strName << " path=" << strPath;

        ui->labelFileName->setText(strFileName);

        //下载
        QFile file(s_tmpDir + "/" + strFileName);
        if (!file.open(QFile::WriteOnly)) {
            return false;
        }
        strUrl = m_settings.strUpdateUrl + "/" + strFileName;
        m_curl.Get(strUrl, &file);
        qDebug() << strUrl;
        file.close();

        ui->progressBar->setValue(++count);
        QCoreApplication::processEvents();
    }

    return true;
}

void CAutoUpdate::slotTimeout()
{
    QString strTmp;

    //获取本地软件信息
    createLocalManifest(m_mapLocalManifest, m_settings.strUpdateDir);
    createRemoteManifest(m_mapLocalManifest, m_settings.strUpdateDir);

    if (!getRemoteManifest(m_mapRemoteManifest)) {
        qDebug() << "获取远端manifest失败";
        QMessageBox::warning(this, "提示", "获取版本信息失败！");
        return;
    }

    //比对版本信息
    FileList diffList;
    compareLocalRemoteManifest(m_mapLocalManifest, m_mapRemoteManifest, diffList);
    if (diffList.empty()) {
        QMessageBox::warning(this, "提示", "软件已经是最新版本，无需更新！");
        return;
    }

    //下载文件
    if (!downloadDiffFiles(diffList)) {
        QMessageBox::warning(this, "提示", "更新失败！");
        return;
    }

    //杀死进程
    strTmp = "taskkill /im FaceHuaMaWT*.exe /f";
    QProcess::execute(strTmp);

    //拷贝更新文件

    //启动软件
}

