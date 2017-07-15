#include "CJson.h"

CJson::CJson()
{
}

CJson::~CJson()
{
}

bool CJson::IsValidJson(const std::string &data)
{
    if (data.empty())
    {
        return false;
    }
    const char *ptr = data.c_str();
    return Count(ptr, '{') == Count(ptr, '}') &&
           Count(ptr, '[') == Count(ptr, ']') &&
           Count(ptr, '"')%2 == 0;
}

int CJson::Count(const char *str, const char ch)
{
    int num = 0;
    char *p = (char *)str;
    while (*p)
    {
        if (ch == *p)
        {
            ++num;
        }
        ++p;
    }
    return num;
}

bool CJson::ParseStringToJsonValue(const std::string &data, Json::Value &value)
{
    if (!IsValidJson(data))
    {
        return false;
    }
    Json::Reader jsonReader;
    value.clear();
    return jsonReader.parse(data, value);
}

void CJson::JsonToMap(const std::string &jsonStr, JsonStringMap &jsonMap)
{
    Json::Value jval;

	if (ParseStringToJsonValue(jsonStr, jval))
    {
        JsonToMap(jval, jsonMap);
    }
}

void CJson::JsonToMap(const Json::Value &jval, JsonStringMap &kvmap)
{
    kvmap.clear();
    if (!jval.isObject())
    {
        return;
    }

	StringVector keyVec = jval.getMemberNames();
	for (StringVector::iterator it = keyVec.begin(); it != keyVec.end(); ++it)
	{
		kvmap[*it] = JsonValueToStringEx(jval[*it]);
	}
}

std::string CJson::JsonValueToStringEx(const Json::Value &value)
{
    char buf[256];
	std::string str;

	switch (value.type())
	{
	case Json::intValue:
		sprintf(buf, "%d", value.asInt());
		str = buf;
		break;
	case Json::uintValue:
		sprintf(buf, "%u", value.asUInt());
		str = buf;
		break;
	case Json::realValue:
		sprintf(buf, "%f", value.asDouble());
		str = buf;
		break;
	case Json::stringValue:
		str = value.asString();
		break;
    case Json::arrayValue:
        str = JsonWriteToString(value);
        break;
    case Json::objectValue:
        str = JsonWriteToString(value);
        break;
	default:
		break;
	}

	return str;
}

std::string CJson::JsonValueToString(const Json::Value &value)
{
    if (!value.isString())
    {
        return std::string();
    }
    return value.asString();
}

int CJson::JsonValueToInt(const Json::Value &value)
{
    if (!value.isInt())
    {
        return 0;
    }
    return value.asInt();
}

std::string CJson::JsonWriteToString(const Json::Value &jv)
{
	if (jv.isNull())
	{
		return "";
	} else {
	    Json::FastWriter jwrite;
	    return jwrite.write(jv);
	}
}

std::string CJson::JsonTest()
{
    Json::Value jval;
    Json::Value jarr;
    Json::Value jopt;
    Json::Value jsql;
    Json::Value jck;
    Json::FastWriter jwrite;

    jval["cmd"] = 2;
    jval["pwd"] = "123456";

    //opt.sql
    jsql["ip"] = "192.168.1.1";
    jsql["port"] = 3306;
    jsql["user"] = "root";
    jsql["pwd"] = "123456";
    jsql["dbname"] = "test";

    //opt.cookiesrv
    jck.append("192.168.1.1:12345");
    jck.append("192.168.1.2:12345");
    jck.append("192.168.1.3:12345");
    jck.append("192.168.1.4:12345");

    //opt
    jopt["sql"] = jsql;
    jopt["cookiesrv"] = jck;

    jval["opt"] = jopt;

    //return jval.toStyledString();
    return JsonWriteToString(jval);
}

