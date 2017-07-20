#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <QApplication>
#include <QWidget>
#include <QLocalServer>
#include <QLocalSocket>
#include <QFile>

class QtSingleApplication : public QApplication
{
    Q_OBJECT

public:
    QtSingleApplication(int &argc, char **argv);
    QtSingleApplication(int &argc, char **argv, bool GUIenabled);
    QtSingleApplication(int &argc, char **argv, Type type);

    bool IsRunning();
    void SetActWindow(QWidget *w);
    QWidget *ActWindow() const;

private slots:
    void SlotRecvConnection();

private:
    void SysInit();

    QWidget *m_pActWindow;
    QLocalServer m_LocalServer;
};

#endif // QTSINGLEAPPLICATION_H
