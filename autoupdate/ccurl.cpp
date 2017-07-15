/***************************************************************************
** Name         : ccurl.cpp
** Author       : snowcicada
** Date         : 20121001
** Description  : Getting jira tasks by year and quarter.
** GitHub       : https://github.com/snowcicada
** E-mail       : snowcicadas#gmail.com (# -> @)
** This file may be redistributed under the terms of the GNU Public License.
***************************************************************************/

/*
重要,必须重新拷贝，否则fieldsInfo转换成char *，可能引起异常现象
*/

#include "ccurl.h"
//模拟浏览器
const char *CCurl::m_UserAgent = "Mozilla/5.0 (Windows NT 6.1; rv:29.0) Gecko/20100101 Firefox/29.0";//"Mozilla/5.0 (Windows NT 5.1; rv:17.0) Gecko/20100101 Firefox/17.0";

CCurl::CCurl()
{
    m_pCurl = NULL;
    m_pConnectionList = NULL;
    Init();
}

CCurl::~CCurl()
{
    Destroy();
}

bool CCurl::Init()
{
    if (!m_pCurl)
    {
        m_pCurl = curl_easy_init();
        if (!m_pCurl)
        {
            qDebug() << "curl_easy_init: " << strerror(errno);
            return false;
        }
        if (!m_pConnectionList)
        {
            //长连接
            m_pConnectionList = curl_slist_append(m_pConnectionList, "Connection: keep-alive");
        }
    }

    return true;
}

void CCurl::Destroy()
{
    if (m_pCurl)
    {
        curl_easy_cleanup(m_pCurl);
        m_pCurl = NULL;
    }
    if (m_pConnectionList)
    {
        curl_slist_free_all(m_pConnectionList);
        m_pConnectionList = NULL;
    }
}

bool CCurl::CheckUrlSpeed(const QString &url, QString &html, double &time)
{
    CURLcode code;

    char *pUrl = Malloc(url.toLatin1().data(), url.size());

    html.clear();
    Reset();
    curl_easy_setopt(m_pCurl, CURLOPT_URL, pUrl);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteToMem);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &html);
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pConnectionList);
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, m_UserAgent);
    /*当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
        如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出*/
    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 15L);

    //proxy
    #if 0
    curl_easy_setopt(m_pCurl, CURLOPT_PROXYUSERPWD, "user_name:password");
    #endif

    code = curl_easy_perform(m_pCurl);
    Free(pUrl);
    if (CURLE_OK != code)
    {
        qDebug() << "curl_easy_perform: " << curl_easy_strerror(code);
        time = 0.0;
        return false;
    }

    curl_easy_getinfo(m_pCurl, CURLINFO_TOTAL_TIME, &time);

    return true;
}

bool CCurl::Get(const QString &visitUrl, QString &htmlStr)
{
    CURLcode code;

    char *pUrl = Malloc(visitUrl.toLatin1().data(), visitUrl.size());
    char *pCkFileName = Malloc(m_CookiesFileName.toAscii().data(), m_CookiesFileName.toAscii().size());

    Reset();
    htmlStr.clear();
    std::string strOutput;
//    stMyString mystr;
//    mystr.str = new char[1024*1024];
//    mystr.len = 0;
    curl_easy_setopt(m_pCurl, CURLOPT_URL, pUrl);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEJAR, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteToMemStd);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &strOutput);
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pConnectionList);
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, m_UserAgent);
    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 15L);
    //debug
//    curl_easy_setopt(m_pCurl, CURLOPT_DEBUGFUNCTION, CurlDebug);//打印完整的调试信息
//    curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);//打印调试信息
//    curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 1);//打印http头

    //proxy
    #if 0
    curl_easy_setopt(m_pCurl, CURLOPT_PROXYUSERPWD, "user_name:password");
    #endif

    code = curl_easy_perform(m_pCurl);
    Free(pUrl);
    Free(pCkFileName);
    QByteArray ba(strOutput.c_str(), strOutput.size());
    htmlStr = ba;

//    FILE* fp = fopen("aaaa.html", "a+");
//    if (fp)
//    {
////        fwrite(strOutput.c_str(), strOutput.size(), 1, fp);
//        fwrite(htmlStr.toStdString().c_str(), htmlStr.size(), 1, fp); ////bytearray编码还是正确的，到了QString就错
//        fclose(fp);
//    }
    if (CURLE_OK != code)
    {
        qDebug() << "curl_easy_perform: " << curl_easy_strerror(code);
        return false;
    }

    return true;
}

bool CCurl::Get(const QString &visitUrl, QFile *fp)
{
    CURLcode code;

    char *pUrl = Malloc(visitUrl.toLatin1().data(), visitUrl.size());
    char *pCkFileName = Malloc(m_CookiesFileName.toAscii().data(), m_CookiesFileName.toAscii().size());

    Reset();
    curl_easy_setopt(m_pCurl, CURLOPT_URL, pUrl);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEJAR, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteToFile);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pConnectionList);
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, m_UserAgent);
    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 15L);
//    curl_easy_setopt(m_pCurl, CURLOPT_DEBUGFUNCTION, CurlDebug);//打印完整的调试信息
//    curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);//打印调试信息

    //proxy
    #if 0
    curl_easy_setopt(m_pCurl, CURLOPT_PROXYUSERPWD, "user_name:password");
    #endif

    code = curl_easy_perform(m_pCurl);
    Free(pUrl);
    Free(pCkFileName);
    if (CURLE_OK != code)
    {
        qDebug() << "curl_easy_perform: " << curl_easy_strerror(code);
        return false;
    }

    return true;
}

bool CCurl::Post(const QString &actionUrl, const QString &fieldsInfo, QString &htmlStr)
{
    CURLcode code;

    char *pUrl = Malloc(actionUrl.toLatin1().data(), actionUrl.size());
    char *pCkFileName = Malloc(m_CookiesFileName.toAscii().data(), m_CookiesFileName.toAscii().size());//有中文
    char *pPostData = Malloc(fieldsInfo.toLatin1().data(), fieldsInfo.size());

    Reset();
    htmlStr.clear();
    curl_easy_setopt(m_pCurl, CURLOPT_URL, pUrl);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEJAR, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, pCkFileName);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteToMem);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &htmlStr);
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, pPostData);//fieldsInfo.toLatin1().data()
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pConnectionList);
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, m_UserAgent);
    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 15L);
//    curl_easy_setopt(m_pCurl, CURLOPT_DEBUGFUNCTION, CurlDebug);//打印完整的调试信息
//    curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);//打印调试信息

    //proxy
    #if 0
    curl_easy_setopt(m_pCurl, CURLOPT_PROXYUSERPWD, "user_name:password");
    #endif

    code = curl_easy_perform(m_pCurl);
    Free(pUrl);
    Free(pCkFileName);
    Free(pPostData);
    if (CURLE_OK != code)
    {
        qDebug() << "curl_easy_perform: " << curl_easy_strerror(code);
        return false;
    }

    return true;
}

void CCurl::Reset()
{
    curl_easy_reset(m_pCurl);
}

void CCurl::SetCookiesFileName(const QString &filename)
{
    m_CookiesFileName = filename;
}

bool CCurl::GlobalInit(long flags)
{
    CURLcode code = curl_global_init(flags);
    if (CURLE_OK != code)
    {
        qDebug() << "curl_global_init: " << curl_easy_strerror(code);
        return false;
    }

    return true;
}

void CCurl::GlobalCleanup()
{
    curl_global_cleanup();
}

size_t CCurl::WriteToMem(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    QString *str = static_cast<QString *>(userdata);
    if (str)
    {
        QByteArray ba(ptr, size * nmemb);
        str->append(ba);
        //str->append((char *)ptr);
    }
    return size*nmemb;
}

size_t CCurl::WriteToMemStd(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string *str = static_cast<std::string *>(userdata);
    if (str)
    {
        str->append((char *)ptr, size * nmemb);
//        char* dst = str->str + str->len;
//        memcpy(dst, ptr, size * nmemb);
//        str->len += size * nmemb;
//        str->append((char *)ptr, size * nmemb);
//        int len = strlen(ptr);
//        int len2 = size * nmemb;
//        if (len != len2)
//        {
//            qDebug() << len << len2;
//        }
//        FILE* fp = fopen("aaaa.html", "a+");
//        if (fp)
//        {
//            fwrite(ptr, size*nmemb, 1, fp);
//            fclose(fp);
//        }
    }
    return size*nmemb;
}

size_t CCurl::WriteToFile(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    return ((QFile *)userdata)->write((char *)ptr, size*nmemb);
}

int CCurl::CurlDebug(CURL *pcurl, curl_infotype itype, char * pData, size_t size, void *)
{
    if(itype == CURLINFO_TEXT)
    {
//        qDebug() << "[TEXT]:" << pData;
    }
    else if(itype == CURLINFO_HEADER_IN)
    {
//        qDebug() << "[HEADER_IN]:" << pData;
    }
    else if(itype == CURLINFO_HEADER_OUT)
    {
        qDebug() << "[HEADER_OUT]:" << pData;
    }
    else if(itype == CURLINFO_DATA_IN)
    {
//        qDebug() << "[DATA_IN]:" << pData;
    }
    else if(itype == CURLINFO_DATA_OUT)
    {
//        qDebug() << "[DATA_OUT]:" << pData;
    }
    return 0;
}

char *CCurl::Malloc(const char *ptr, int size)
{
    char *buf = new char[size+1];
    qMemCopy(buf, ptr, size);
    buf[size] = 0;
    return buf;
}

void CCurl::Free(char *ptr)
{
    if (ptr)
    {
        delete [] ptr;
        ptr = NULL;
    }
}
