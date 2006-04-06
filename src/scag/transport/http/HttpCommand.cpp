#include <string>
#include <iconv.h>
#include "util/Exception.hpp"
#include "HttpCommand.h"
#include "HttpParser.h"

#define ICONV_BLOCK_SIZE 32
#define CRLF "\r\n"
#define SP ' '

using namespace std;

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

HttpCommand::~HttpCommand()
{
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
    queryParameters[paramName.c_str()] = _paramValue;
}

const std::string& HttpRequest::getQueryParameter(const std::string& paramName)
{
    const std::string *valptr = queryParameters.GetPtr(paramName.c_str());
    
    if (valptr) {
        paramValue.assign(*valptr);
        HttpParser::urlDecode(paramValue);
        return paramValue;
    }
    else {
        return empty;
    }
}

const wstring& HttpCommand::getMessageText()
{
    if (textContent.empty() &&
        !strncasecmp(getHeaderField(content_type_field).c_str(), "text/", 5))
    {
        size_t result = 0;
        size_t inbytesleft;
        char *outbufptr;
        const char *inbufptr = getMessageContent(inbytesleft);
        size_t outbufsize;
        size_t outbytesleft;
        iconv_t cd = iconv_open("wchar_t", charset.c_str());
        wchar_t *outbuf;

        if (cd == (iconv_t)(-1))
            throw Exception("iconv_open() failed");

        if (inbytesleft) {
            if (cd == (iconv_t)(-1))
                throw Exception("iconv_open() failed");
        
            outbufsize = inbytesleft;
            outbytesleft = outbufsize * sizeof(wchar_t);
            outbuf = new wchar_t[outbufsize];
            outbufptr = (char *)outbuf;

            result = iconv(cd, &inbufptr, &inbytesleft, &outbufptr, &outbytesleft);
            
            iconv_close(cd);

            textContent.assign(outbuf, outbufsize - outbytesleft / sizeof(wchar_t));
            delete outbuf;

            if (result == (size_t)(-1))
                throw Exception("iconv() failed");
        }
    }
        
    return textContent;
}

bool HttpCommand::setMessageText(const wstring& text)
{
    textContent.clear();   

    const std::string &content_type = getHeaderField(content_type_field);

    if (content_type.empty() ||
        !strncasecmp(content_type.c_str(), "text/", 5))
    {  
        size_t result = 0;
        size_t outbytesleft;    
        size_t inbytesleft = text.size() * sizeof(wchar_t);
        char *outbufptr;
        const char *inbufptr = (char *)text.data();
        iconv_t cd = iconv_open(charset.c_str(), "wchar_t");

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

void HttpRequest::serialize()
{
    if (headers.empty()) {
        char *keystr;
        std::string *valptr;
        char buf[20];
        buf[19] = 0;
        
        //headers.reserve(totalHeadersSize);
        
        headers = httpMethodNames[httpMethod];
        headers += SP;
        headers += sitePath;

#ifdef SESSION_ID_ENABLED
        headers += '/';
        headers += getAbonent();
        headers += '_';
        headers += lltostr(getUSR(), buf + 19);
#endif        
        if (!(siteQuery.empty() || httpMethod == POST)) {
            headers += '?';
            headers += siteQuery;
        }
        
        headers += SP;
        headers += httpVersion;
        headers += CRLF;

        setHeaderField(connection_field, close);
        
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
    
    switch (s) {
    case 405:
        sl = "Method Not Allowed";
        break;
    case 500:
        sl = "Internal Server Error";
        break;
    case 503:
        sl = "Service Unavailable";
        break;  
    default:
        sl = "HTTP Transport Error";
        break;
    }

    status = s;
    statusLine = sl;
    
    if (httpVersion.empty())
        httpVersion = "HTTP/1.1";
            
    headerFields.Assign(StringHash());
    content.SetPos(0);
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
