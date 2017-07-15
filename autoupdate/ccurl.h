/***************************************************************************
** Name         : ccurl.h
** Author       : snowcicada
** Date         : 20121001
** Description  : Getting jira tasks by year and quarter.
** GitHub       : https://github.com/snowcicada
** E-mail       : snowcicadas#gmail.com (# -> @)
** This file may be redistributed under the terms of the GNU Public License.
***************************************************************************/

#ifndef CCURL_H
#define CCURL_H

#include <QtCore>
#include "curl/curl.h"

class CCurl
{
public:
    CCurl();
    ~CCurl();

    bool Init();
    void Destroy();
    bool CheckUrlSpeed(const QString &url, QString &html, double &time);
    bool Get(const QString &visitUrl, QString &htmlStr);
    bool Get(const QString &visitUrl, QFile *fp);
    bool Post(const QString &actionUrl, const QString &fieldsInfo, QString &htmlStr);
    void Reset();
    void SetCookiesFileName(const QString &filename);

    static bool GlobalInit(long flags = CURL_GLOBAL_ALL);
    static void GlobalCleanup();
    static size_t WriteToMem(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t WriteToMemStd(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t WriteToFile(char *ptr, size_t size, size_t nmemb, void *userdata);
    static int CurlDebug(CURL *pcurl, curl_infotype itype, char * pData, size_t size, void *);

private:
    char *Malloc(const char *ptr, int size);
    void  Free(char *ptr);

private:
    CURL    *m_pCurl;
    struct curl_slist *m_pConnectionList;
    QString  m_CookiesFileName;
    static const char *m_UserAgent;
};

#endif // CCURL_H
