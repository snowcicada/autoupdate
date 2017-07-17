#include "cautoupdate.h"
#include "ui_cautoupdate.h"
#include "jsoncpp/CJson.h"
#include <QCryptographicHash>

#define SET_UPDATE          "config.ini"
#define APP_TITLE           "华码科技自动更新 v1.1"
#define APP_NAME            "autoupdate.exe"
#define PROJECT_MANIFEST    "project.manifest"
#define TMP_DIR             "tmp"

CAutoUpdate::CAutoUpdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAutoUpdate)
{
    ui->setupUi(this);

    initUi();
    readSettings();

//    createRemoteManifest(m_mapLocalManifest, m_settings.strUpdateDir);

    QTimer::singleShot(1, this, SLOT(slotTimeout()));
}

CAutoUpdate::~CAutoUpdate()
{
    delete ui;
}

void CAutoUpdate::closeEvent(QCloseEvent *e)
{
    this->hide();
    e->ignore();
    if (m_pSysTrayIcon)
    {
        m_pSysTrayIcon->showMessage(tr("提示"), tr("软件将在后台更新！"), QSystemTrayIcon::Information);
    }
}

void CAutoUpdate::initUi()
{
    m_nSize = 0;
    ui->labelSize->setText("0B");

    setWindowIcon(QIcon(":/images/logo.ico"));
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    setFixedSize(width(), height());
    setWindowTitle(APP_TITLE);
    ui->labelFileName->clear();
    ui->pgsBarUpdate->setValue(0);
    ui->pgsBarUpdate->setMaximum(1);
    ui->pgsBarCopy->setValue(0);
    ui->pgsBarCopy->setMaximum(1);

    m_pActQuit = new QAction(QIcon(":/images/exit.png"), tr("退出"), this);
    connect(m_pActQuit, SIGNAL(triggered()), this, SLOT(slotActQuit()));

    m_pSysTrayMenu = new QMenu(this);
    m_pSysTrayMenu->addAction(m_pActQuit);

    m_pSysTrayIcon = new QSystemTrayIcon(QIcon(":/images/logo.ico"), this);
    m_pSysTrayIcon->setToolTip(APP_TITLE);
    m_pSysTrayIcon->setContextMenu(m_pSysTrayMenu);
    m_pSysTrayIcon->show();
    connect(m_pSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    connect(&m_curl, SIGNAL(signalSize(int)), this ,SLOT(slotCurlSize(int)));
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
        m_settings.strUpdateDir = "..";
        set.setValue(strKey, m_settings.strUpdateDir);
    }

    strKey = "update/not_update_dirs";
    if (set.contains(strKey)) {
        strTmp = set.value(strKey).toString();
        m_settings.strNotUpdateDirList = strTmp.split(';');
    } else {
        strTmp = "id_logs;";
        set.setValue(strKey, strTmp);
        m_settings.strNotUpdateDirList = strTmp.split(';');
    }

    strKey = "update/not_update_files";
    if (set.contains(strKey)) {
        strTmp = set.value(strKey).toString();
        m_settings.strNotUpdateFileList = strTmp.split(';');
    } else {
        strTmp = "cardreadlog.txt;CollectConfig.ini;IDInfoLog.txt;Log.dat;upload.txt;";
        set.setValue(strKey, strTmp);
        m_settings.strNotUpdateFileList = strTmp.split(';');
    }

    strKey = "update/update_url";
    if (set.contains(strKey)) {
        m_settings.strUpdateUrl = set.value(strKey).toString();
    } else {
        m_settings.strUpdateUrl = "";
        set.setValue(strKey, "http://127.0.0.1:8080");
        QMessageBox::warning(this, "提示", "请配置更新地址update_url！");
        return;
    }

    strKey = "update/app";
    if (set.contains(strKey)) {
        m_settings.strApp = set.value(strKey).toString();
    } else {
        m_settings.strApp = "FaceHuaMaWT.exe";
        set.setValue(strKey, m_settings.strApp);
    }

    strKey = "update/update_interval";
    if (set.contains(strKey)) {
        m_settings.nUpdateInterval = set.value(strKey).toInt();
    } else {
        m_settings.nUpdateInterval = 3600;
        set.setValue(strKey, m_settings.nUpdateInterval);
    }
}

bool CAutoUpdate::createLocalManifest(QQMAP &mapManifest, const QString &strPath)
{
    m_fileInfoList.clear();
    mapManifest.clear();

    searchFile(m_fileInfoList, strPath);

    QString strFileName, strMd5;
    foreach (QFileInfo info, m_fileInfoList) {
        strFileName = info.filePath().replace(m_settings.strUpdateDir + "/", "");
        QFile file(info.filePath());
        if (file.open(QFile::ReadOnly)) {
            strMd5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        } else {
            qDebug() << "读取文件失败!";
            mapManifest.clear();
            return false;
        }
        file.close();
        mapManifest[strFileName] = strMd5;
        QCoreApplication::processEvents();
//        qDebug() << strFileName << " -> " << strMd5;
    }

    return true;
}

bool CAutoUpdate::createRemoteManifest(QQMAP &mapManifest, const QString &strPath)
{
    m_fileInfoList.clear();
    mapManifest.clear();

    searchFile(m_fileInfoList, strPath);

    Json::Value jsonVal;
    QString strFileName, strMd5;
    foreach (QFileInfo info, m_fileInfoList) {
        strFileName = info.filePath().replace(m_settings.strUpdateDir + "/", "");
        QFile file(info.filePath());
        if (file.open(QFile::ReadOnly)) {
            strMd5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        } else {
            qDebug() << "读取文件失败!";
            mapManifest.clear();
            return false;
        }
        file.close();
        mapManifest[strFileName] = strMd5;
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
        if (strCurrentPath == info.fileName() || info.fileName() == "." || info.fileName() == "..") {
            continue;
        }

        if (info.isFile() && m_settings.strNotUpdateFileList.indexOf(info.fileName()) == -1) {
            infoList.append(info);
        } else if (info.isDir() && m_settings.strNotUpdateDirList.indexOf(info.fileName()) == -1) {
            searchFile(infoList, info.filePath());
        }
    }
}

void CAutoUpdate::searchFileEx(QFileInfoList &infoList, const QString &strPath)
{
    QString strCurrentPath = getCurrentDirName();
    QDir dir(strPath);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo info, fileList) {
        if (info.fileName() == "." || info.fileName() == "..") {
            continue;
        }

        if (info.isFile()) {
            infoList.append(info);
        } else if (info.isDir() && strCurrentPath != info.fileName()) {
            searchFileEx(infoList, info.filePath());
        }
    }
}

QString CAutoUpdate::getCurrentDirName()
{
    QString strPath = qApp->applicationDirPath();
    int index = strPath.lastIndexOf('/');
    QString strCurrentDir = strPath.right(strPath.length() - index - 1);
    return strCurrentDir;

//    QString strPath = QDir::currentPath();
//    QStringList strList = strPath.split('/');
//    if (!strList.isEmpty()) {
//        return strList.last();
//    }
//    return "";
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
//    fileList.push_back("THFaceImage.dll");
    for (auto& it : remote) {
        auto fit = local.find(it.first);
        fileList.push_back(it.first);
        //test
//        if (local.end() == fit) {
//            //新增文件
//            fileList.push_back(it.first);
//        } else if (fit->second != it.second) {
//            //md5不同
//            fileList.push_back(it.first);
//        }
    }
}

bool CAutoUpdate::downloadDiffFiles(const FileList &fileList)
{
    const QString s_tmpDir = TMP_DIR;
    QString strUrl, strTmp;

    //每次重新创建目录
    QDir dir = QDir::current();
    if (dir.exists(s_tmpDir)) {
        removeAllFiles(s_tmpDir);
    }
    dir.mkdir(s_tmpDir);

    int index = 0, count = 0;
    QString strFileName, strName, strPath;
    ui->pgsBarUpdate->setMaximum(fileList.size());
    ui->pgsBarUpdate->setValue(0);
    ui->labelSize->setText("0B");
    m_nSize = 0;
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
            dir.mkpath(s_tmpDir + "/" + strPath);
//            qDebug() << s_tmpDir + "/" + strPath;
        }
//        qDebug() << "name=" << strName << " path=" << strPath;

        ui->labelFileName->setText(strFileName);

        //下载
        strTmp = s_tmpDir + "/" + strFileName;
        QFile file(strTmp);
        if (!file.open(QFile::WriteOnly)) {
            return false;
        }
        strUrl = m_settings.strUpdateUrl + "/" + strFileName;
        m_curl.Get(strUrl, &file);
        file.close();

        ui->pgsBarUpdate->setValue(++count);
        QCoreApplication::processEvents();
    }

    return true;
}

QString CAutoUpdate::removeSetComment(const QString &str)
{
    int index = str.indexOf('#');
    return str.left(index);
}

bool CAutoUpdate::canUpdate()
{
    if (m_settings.strUpdateUrl.isEmpty()) {
        QMessageBox::warning(this, "提示", "请设置更新地址update_url");
        return false;
    }

    return true;
}

void CAutoUpdate::removeAllFiles(const QString &strPath)
{
    QDir dir(strPath);
    QFileInfoList fileInfoList = dir.entryInfoList();
    foreach (QFileInfo info, fileInfoList) {
        if (info.fileName() == "." || info.fileName() == "..") {
            continue;
        }

        if (info.isFile()) {
            QFile::remove(info.filePath());
        } else if (info.isDir()) {
            removeAllFiles(info.filePath());
        }
    }
}

void CAutoUpdate::copyAllFiles(const QString &strSrcPath, const QString &strDstPath)
{
    QString strTmp, strPre;

    //src
    QQMAP kvMapSrc;
    QFileInfoList fileInfoListSrc;
    searchFileEx(fileInfoListSrc, strSrcPath);
    strPre = strSrcPath + "/";
    foreach (QFileInfo info, fileInfoListSrc) {
        strTmp = info.filePath().replace(strPre, "");
        kvMapSrc[strTmp] = info.absoluteFilePath();
//        qDebug() << "src " << strTmp << " -> " << info.absoluteFilePath();
    }

    QQMAP kvMapDst;
    QFileInfoList fileInfoListDst;
    searchFileEx(fileInfoListDst, strDstPath);
    strPre = strDstPath + "/";
    foreach (QFileInfo info, fileInfoListDst) {
        strTmp = info.filePath().replace(strPre, "");
        kvMapDst[strTmp] = info.absoluteFilePath();
//        qDebug() << "dst " << strTmp << " -> " << info.absoluteFilePath();
    }

    //从更新文件开始拷贝过去
    int count = 0;
    QString strSimplePath, strAbsolutePath, strAbsolutePathDst;
    ui->pgsBarCopy->setMaximum(kvMapSrc.size());
    for (auto& it : kvMapSrc) {
        strSimplePath = it.first;
        strAbsolutePath = it.second;
        strAbsolutePathDst = kvMapDst[strSimplePath];
        if (QFile::exists(strAbsolutePath)) {
            QFile::remove(strAbsolutePathDst);
//            qDebug() << "remove " << strAbsolutePathDst << QFile::remove(strAbsolutePathDst);
        }
        QFile::copy(strAbsolutePath, strAbsolutePathDst);
//        qDebug() << "copy " << strSimplePath << QFile::copy(strAbsolutePath, strAbsolutePathDst);
        ui->pgsBarCopy->setValue(++count);
        ui->labelFileName->setText(strSimplePath);
    }
}

QString CAutoUpdate::convertFileSize(int size)
{
    static const int s_b = 1024;
    static const int s_kb = 1024 * 1024;
    static const int s_mb = 1024 * 1024 * 1024;

    QString strSize;
    if (size < s_b) {
        strSize = tr("%1B").arg(size);
    } else if (size < s_kb) {
        strSize = tr("%1KB").arg(1.0 * size / s_b, 0, 'f', 2);
    } else if (size < s_mb) {
        strSize = tr("%1MB").arg(1.0 * size / s_kb, 0, 'f', 2);
    }

    return strSize;
}

void CAutoUpdate::slotTimeout()
{
    m_timer.stop();

    ui->labelFileName->setText("检查更新");

    if (!canUpdate()) {
        m_timer.start(m_settings.nUpdateInterval * 1000);
        return;
    }

    QString strTmp;

    //获取本地软件信息
    createLocalManifest(m_mapLocalManifest, m_settings.strUpdateDir);
//    createRemoteManifest(m_mapRemoteManifest, m_settings.strUpdateUrl + "/" + PROJECT_MANIFEST);

    if (!getRemoteManifest(m_mapRemoteManifest)) {
        m_pSysTrayIcon->showMessage("提示", "获取版本信息失败！", QSystemTrayIcon::Information);
        m_timer.start(m_settings.nUpdateInterval * 1000);
        return;
    }

    //比对版本信息
    FileList diffList;
    compareLocalRemoteManifest(m_mapLocalManifest, m_mapRemoteManifest, diffList);
    if (diffList.empty()) {
        ui->labelFileName->setText("软件已经是最新版本！");
        m_timer.start(m_settings.nUpdateInterval * 1000);
        return;
    }

    //下载文件
    m_pSysTrayIcon->showMessage("提示", "开始下载更新文件。。。", QSystemTrayIcon::Information);
    if (!downloadDiffFiles(diffList)) {
        m_pSysTrayIcon->showMessage("提示", "下载更新文件失败！", QSystemTrayIcon::Information);
        m_timer.start(m_settings.nUpdateInterval * 1000);
        return;
    }

    //杀死进程
    strTmp = tr("taskkill /im %1* /f").arg(m_settings.strApp);
    QProcess::execute(strTmp);

    //拷贝更新文件
    copyAllFiles(TMP_DIR, m_settings.strUpdateDir);

    //启动软件,先切到目标目录启动，再切回去，由于对方的软件含有相对路径操作，所以要这么处理
    QString strCurrentDir = qApp->applicationDirPath();
    QDir::setCurrent(m_settings.strUpdateDir + "/");
    QProcess::startDetached(m_settings.strApp);
    QDir::setCurrent(strCurrentDir);

    //结束
    ui->labelFileName->setText("更新完成");
    m_pSysTrayIcon->showMessage("提示", "更新完成！", QSystemTrayIcon::Information);

    //继续开启下一次更新
    m_timer.start(m_settings.nUpdateInterval * 1000);
}

void CAutoUpdate::slotActQuit()
{
    if (QMessageBox::warning(this, tr("退出"), tr("确定退出自动更新？"), QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
    {
        qApp->quit();
    }
}

void CAutoUpdate::slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason)
    {
        this->activateWindow();
        this->showNormal();
    }
}

void CAutoUpdate::slotCurlSize(int size)
{
    m_nSize += size;
    ui->labelSize->setText(convertFileSize(m_nSize));
    QCoreApplication::processEvents();
}

