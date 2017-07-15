#pragma once

#include "reader.h"
#include "writer.h"
#include "value.h"
#include <list>

typedef std::map<std::string, std::string> JsonStringMap;
typedef std::list<std::string> StringList;
typedef std::vector<std::string> StringVector;

class CJson
{
public:
    CJson();
    virtual ~CJson();

    static bool         ParseStringToJsonValue(const std::string &data, Json::Value &value);
    static void         JsonToMap(const std::string &jsonStr, JsonStringMap &jsonMap);
    static void         JsonToMap(const Json::Value &jval, JsonStringMap &kvmap);
    static std::string  JsonValueToStringEx(const Json::Value &value);//把所有json数据转化成string
    static std::string  JsonValueToString(const Json::Value &value);
    static int          JsonValueToInt(const Json::Value &value);
    static std::string	JsonWriteToString(const Json::Value &jv);
    static std::string  JsonTest();

private:
    static bool         IsValidJson(const std::string &data);
    static int          Count(const char *str, const char ch);
};

