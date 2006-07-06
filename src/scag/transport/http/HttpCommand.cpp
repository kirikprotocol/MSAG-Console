#include <string>
#include <iconv.h>
#include "util/Exception.hpp"
#include "HttpCommand.h"
#include "HttpParser.h"

#define ICONV_BLOCK_SIZE 32
#define CRLF "\r\n"
#define SP ' '

//using namespace std;

namespace scag { namespace transport { namespace http
{
using smsc::util::Exception;

const char *HttpRequest::httpMethodNames[9] = {
    NULL,
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT",
    "OPTIONS"    
};

static const std::string content_length_field(CONTENT_LENGTH_FIELD);
static const std::string content_type_field(CONTENT_TYPE_FIELD);
static const std::string connection_field(CONNECTION_FIELD);
static const std::string close("close");
static const std::string empty;        

void Cookie::serialize(std::string& s, bool set)
{
    if(name.length())
    {
        s += name;
        if(value.length())
        {
            s += "=";
            s += value;
        }
        s += ";";
    }

    char *keystr;
    std::string *valptr;

    params.First();
    while (params.Next(keystr, valptr))
    {
        if(!set)
            s += '$';
        s += keystr;
        s += "=";
        s += *valptr;
        s += ";";
    }
}

const std::string& Cookie::getParam(const char * nm)
{
    std::string *a = params.GetPtr(nm);
    if(a)
        return *a;
    return empty;
}

void Cookie::setParam(const char* nm, std::string& val)
{
    params.Insert(nm, val);
}

void Cookie::setParam(const char* nm, uint32_t ncnt, const char* val, uint32_t vcnt)
{
    std::string n, v;
    n.assign(nm, ncnt);

    for (unsigned int i = 0; i < n.length(); ++i)
        n[i] = tolower(n[i]);

    v.assign(val, vcnt);
    params.Insert(n.c_str(), v);
}

HttpCommand::~HttpCommand()
{
    char *keystr;
    Cookie **valptr;

    cookies.First();
    while (cookies.Next(keystr, valptr))
        if(*valptr) delete *valptr;
}

Cookie* HttpCommand::getCookie(const std::string& name)
{
    Cookie **c = cookies.GetPtr(name.c_str());
    if(c)
        return *c;
    return NULL;
}

Cookie* HttpCommand::setCookie(const std::string& nm, const std::string& val)
{
    Cookie *c = getCookie(nm);
    if(c == NULL)
    {
        c = new Cookie;
        c->name = nm;
        cookies.Insert(nm.c_str(), c);
    }
    c->value = val;
    return c;
}

// virtual functions {
TransportType HttpCommand::getType() const
{
    return HTTP;
}
int HttpCommand::getRuleId() const
{
    return trc.ruleId;
}
void HttpCommand::setRuleId(int ruleId)
{
    trc.ruleId = ruleId;
}
int HttpCommand::getServiceId() const
{
    return trc.serviceId;
}
void HttpCommand::setServiceId(int serviceId)
{
    trc.serviceId = serviceId;
}
int HttpCommand::getProviderId() const
{
    return trc.providerId;
}
void HttpCommand::setProviderId(int providerId)
{
    trc.providerId = providerId;
}
int64_t HttpCommand::getOperationId() const
{
    return trc.operationId;
}
void HttpCommand::setOperationId(int64_t id)
{
    trc.operationId = id;
}
const std::string& HttpCommand::getRouteId() const
{
    return trc.routeId;
}
void HttpCommand::setRouteId(const std::string& routeId)
{
    trc.routeId = routeId;
}
// } virtual functions

bool StringHashIterator::next()
{
    char *keystr;
    std::string *valptr;

    if (masterHash.Next(keystr, valptr)) {
        key.assign(keystr);
        return true;
    }
    else {
        key.clear();
        return false;
    }
}

HttpCommand::FieldIterator& HttpCommand::getHeaderFieldNames()
{
    headerFields.First();
    return headerFieldsIterator;
}

const std::string& HttpCommand::getHeaderField(const std::string& fieldName)
{
    const std::string *valptr = headerFields.GetPtr(fieldName.c_str());

    return valptr ? *valptr : empty;
}

void HttpCommand::setHeaderField(const std::string& fieldName,
                                const std::string& fieldValue)
{
    headerFields[fieldName.c_str()] = fieldValue;
}

HttpRequest::ParameterIterator& HttpRequest::getQueryParameterNames()
{
    queryParameters.First();
    return queryParametersIterator;
}

void HttpRequest::setQueryParameter(const std::string& paramName,
                                const std::string& _paramValue)
{
    std::string s = _paramValue;
    HttpParser::urlEncode(s);
    queryParameters[paramName.c_str()] = s;
}

void HttpRequest::setQueryParameterEncoded(const std::string& paramName,
                                const std::string& _paramValue)
{
    queryParameters[paramName.c_str()] = _paramValue;
}

const std::string& HttpRequest::getQueryParameter(const std::string& paramName)
{
    const std::string *valptr = queryParameters.GetPtr(paramName.c_str());
    
    if (valptr) {
        paramValue.assign(*valptr);
        if(HttpParser::urlDecode(paramValue) != OK)
            return empty;
        return paramValue;
    }
    else {
        return empty;
    }
}

const std::string& HttpCommand::getMessageText()
{
    if (textContent.empty() &&
        !strncasecmp(getHeaderField(content_type_field).c_str(), "text/", 5))
    {
        size_t result = 0;
        size_t inbytesleft;
        char *outbufptr;
        const char *inbufptr = getMessageContent(inbytesleft);
        size_t outbytesleft;
        iconv_t cd = iconv_open("utf8", charset.c_str());

        if (cd == (iconv_t)(-1))
            throw Exception("iconv_open() failed");

        if (inbytesleft) {
            if (cd == (iconv_t)(-1))
                throw Exception("iconv_open() failed");
        
            TmpBuf<char, 2048> buf(2048);
            buf.SetPos(0);
            while (inbytesleft) {
                buf.setSize(buf.GetPos() + ICONV_BLOCK_SIZE);
                outbufptr = buf.GetCurPtr();
                outbytesleft = ICONV_BLOCK_SIZE;

                result = iconv(cd, &inbufptr, &inbytesleft, &outbufptr, &outbytesleft);
                if (result == (size_t)(-1) && errno != E2BIG)
                    break;
                
                buf.SetPos(buf.GetPos() + ICONV_BLOCK_SIZE - outbytesleft);
            }

            iconv_close(cd);

            textContent.assign(buf.get(), buf.GetPos());

            if (result == (size_t)(-1))
                throw Exception("iconv() failed");
        }
    }
        
    return textContent;
}

bool HttpCommand::setMessageText(const std::string& text)
{
    textContent.clear();   

    const std::string &content_type = getHeaderField(content_type_field);

    if (content_type.empty() ||
        !strncasecmp(content_type.c_str(), "text/", 5))
    {  
        size_t result = 0;
        size_t outbytesleft;    
        size_t inbytesleft = text.size();
        char *outbufptr;
        const char *inbufptr = (char *)text.data();
        iconv_t cd = iconv_open(charset.c_str(), "utf8");

        if (cd == (iconv_t)(-1))
            throw Exception("iconv_open() failed");

        content.SetPos(0);
        while (inbytesleft) {
            content.setSize(content.GetPos() + ICONV_BLOCK_SIZE);
            outbufptr = content.GetCurPtr();
            outbytesleft = ICONV_BLOCK_SIZE;

            result = iconv(cd, &inbufptr, &inbytesleft, &outbufptr, &outbytesleft);
            if (result == (size_t)(-1) && errno != E2BIG)
                break;
                
            content.SetPos(content.GetPos() + ICONV_BLOCK_SIZE - outbytesleft);
        }

        iconv_close(cd);

        if (result == (size_t)(-1))
            throw Exception("iconv() failed");

        if (contentLength >= 0) {
            setContentLength(content.GetPos());
            setLengthField(content.GetPos());
        }

        return true;
    }
    
    return false;
}

const uint8_t* HttpCommand::getMessageBinary(int& length)
{   
    return (uint8_t*)getMessageContent((unsigned int &)length);
}

void HttpCommand::setLengthField(unsigned int length)
{    
    char buf[11];    
    buf[10] = 0;

    setHeaderField(content_length_field, std::string(lltostr(length, buf + 10)));
}

void HttpCommand::setMessageBinary(uint8_t* body, int length, const std::string& cp)
{
    setContentLength(length);
    content.SetPos(0);
    content.Append((char *)body, length);
    
    setLengthField(length);
    setHeaderField(content_type_field, cp);

    textContent.clear();
}

void HttpRequest::serializeQuery(std::string& s)
{
    bool n = false;
    char *keystr;
    std::string *valptr;
    std::string t;

    queryParameters.First();
    while (queryParameters.Next(keystr, valptr))
    {
        if(n)
            s += "&";
        else
            n = true;
        s += keystr;
        s += "=";
        t = *valptr;
        s += t;
    }
}

const std::string& HttpRequest::serialize()
{
//    if (headers.empty()) {
        char *keystr;
        std::string *valptr;
        char buf[20];
        buf[19] = 0;
        
        //headers.reserve(totalHeadersSize);
        
        headers = httpMethodNames[httpMethod];
        headers += SP;
        headers += sitePath;
        headers += siteFileName;

        if (queryParameters.GetCount() && httpMethod == GET) {
            headers += '?';
            serializeQuery(headers);
        }
        
        headers += SP;
        headers += httpVersion;
        headers += CRLF;

        setHeaderField(connection_field, close);

        if(httpMethod == POST && !strcmp(contentType.c_str(), "application/x-www-form-url-encoded"))
        {
            std::string cnt;
            serializeQuery(cnt);
            content.SetPos(0);
            content.Append(cnt.c_str(), cnt.length());
            setContentLength(cnt.length());
        }
        
        if(cookies.GetCount())
        {
            headers += "cookie: ";
            defCookie.serialize(headers, false);
            Cookie** ck;
            cookies.First();
            while (cookies.Next(keystr, ck)) {
                (*ck)->serialize(headers, false);
            }
            headers += CRLF;
        }

        setHeaderField("host", site + ((sitePort != 80) ? ':' + lltostr(sitePort, buf + 19) : ""));

        headerFields.First();
        while (headerFields.Next(keystr, valptr)) {
            headers += keystr;
            headers += ':';
            headers += *valptr;
            headers += CRLF;
        }
        
        headers += CRLF;
        return headers;
//    }
}

void HttpResponse::serialize()
{
    if (headers.empty()) {
        char *keystr;
        std::string *valptr;
        char buf[20];
        buf[19] = 0;
        
        //headers.reserve(totalHeadersSize);
        
        headers = httpVersion;
        headers += SP;

        headers += lltostr(status, buf + 19);
        headers += SP;
        headers += statusLine;
        headers += CRLF;

        setHeaderField(connection_field, close);
        
        if(cookies.GetCount())
        {
            headers += "set-cookie: ";
            defCookie.serialize(headers, true);
            Cookie** ck;
            cookies.First();
            while (cookies.Next(keystr, ck)) {
                (*ck)->serialize(headers, true);
            }
            headers += CRLF;
        }

        headerFields.First();
        while (headerFields.Next(keystr, valptr)) {
            headers += keystr;
            headers += ':';
            headers += *valptr;
            headers += CRLF;
        }
        
        headers += CRLF;
    }
}

void HttpResponse::fillFakeResponse(int s)
{
    const char *sl;
    int len;
    
    switch (s) {
    case 405:
        sl = "Method Not Allowed";
        len = sizeof("Method Not Allowed");
        break;
    case 500:
        sl = "Internal Server Error";
        len = sizeof("Internal Server Error");
        break;
    case 503:
        sl = "Service Unavailable";
        len = sizeof("Service Unavailable");
        break;  
    default:
        sl = "HTTP Transport Error";
        len = sizeof("HTTP Transport Error");
        break;
    }

    status = s;
    statusLine = sl;
    
    if (httpVersion.empty())
        httpVersion = "HTTP/1.1";
            
    headerFields.Assign(StringHash());
    content.SetPos(0);
    content.Append("<html><body><h1>", 16);
    content.Append(sl, len);
    content.Append("</h1></body></html>", 19);
    setLengthField(content.GetPos());
}

bool HttpResponse::isResponse()
{
    return true;
}

bool HttpRequest::isResponse()
{
    return false;
}

}}}
