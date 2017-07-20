#include "qtsingleapplication.h"

QtSingleApplication::QtSingleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    SysInit();
}

QtSingleApplication::QtSingleApplication(int &argc, char **argv, bool GUIenabled)
    : QApplication(argc, argv, GUIenabled)
{
    SysInit();
}

QtSingleApplication::QtSingleApplication(int &argc, char **argv, Type type)
    : QApplication(argc, argv, type)
{
    SysInit();
}

bool QtSingleApplication::IsRunning()
{
    QString appName("autoupdate.exe");
    QLocalSocket socket;

    setApplicationName(appName);

    socket.connectToServer(appName);
    if (socket.waitForConnected())
    {
        socket.write(appName.toLatin1());
        socket.waitForBytesWritten();
        return true;
    }

    if (!m_LocalServer.listen(appName))
    {
        if (m_LocalServer.serverError() == QAbstractSocket::AddressInUseError
                && QFile::exists(m_LocalServer.fullServerName()))
        {
            QFile::remove(m_LocalServer.fullServerName());
            m_LocalServer.listen(appName);
        }
    }

    return false;
}

void QtSingleApplication::SetActWindow(QWidget *w)
{
    m_pActWindow = w;
}

QWidget *QtSingleApplication::ActWindow() const
{
    return m_pActWindow;
}

void QtSingleApplication::SlotRecvConnection()
{
    QLocalSocket *socket = m_LocalServer.nextPendingConnection();
    if (!socket)
    {
        return;
    }

    if (socket->waitForReadyRead()
            && socket->readAll() == applicationName()
            && m_pActWindow)
    {
        m_pActWindow->activateWindow();
        m_pActWindow->showNormal();
    }
}

void QtSingleApplication::SysInit()
{
    m_pActWindow = NULL;
    connect(&m_LocalServer, SIGNAL(newConnection()), this, SLOT(SlotRecvConnection()));
}
