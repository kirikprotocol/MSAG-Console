#include <string>
#include <iconv.h>
#include "util/Exception.hpp"
#include "HttpCommand2.h"
#include "HttpParser.h"
#include "scag/util/encodings/Encodings.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/lltostr.h"

#include "util/debug.h"

#define ICONV_BLOCK_SIZE 32
#define CRLF "\r\n"
#define SP ' '

//using namespace std;

namespace scag2 {
namespace transport {
namespace http {

using smsc::util::Exception;
using exceptions::SCAGException;
using util::encodings::Convertor;

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
static const std::string transfer_encoding(TRANSFER_ENCODING_FIELD);
static const std::string content_encoding(CONTENT_ENCODING_FIELD);
static const std::string accept_encoding(ACCEPT_ENCODING_FIELD);
static const std::string close("close");
static const std::string empty;        


const char* commandIdName( int id )
{
    switch (id) {
    case HTTP_REQUEST: return "HTTP_REQUEST";
    case HTTP_RESPONSE: return "HTTP_RESPONSE";
    case HTTP_DELIVERY: return "HTTP_DELIVERY";
    default:
        return "???";
    };
}


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

//    for (unsigned int i = 0; i < n.length(); ++i)
//        n[i] = tolower(n[i]);

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

void HttpRequest::delCookie(const std::string& name) {
  Cookie **c = cookies.GetPtr(name.c_str());
  if (!c) {
    return;
  }
  if (*c) {
    delete *c;
  }
  cookies.Delete(name.c_str());
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


void HttpCommand::print( util::Print& p ) const
{
    if ( ! p.enabled() ) return;
    p.print( "httpcmd=%p session=%p type=%d(%s) serial=%d svc=%d opid=%d %s(%s)->%s",
             this, session_, command_id, commandIdName(command_id),
             serial_,
             getServiceId(), getOperationId() );
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
opid_type HttpCommand::getOperationId() const
{
    return trc.operationId;
}
void HttpCommand::setOperationId(opid_type id)
{
    trc.operationId = id;
}
uint32_t HttpCommand::getRouteId() const
{
    return trc.routeId;
}
void HttpCommand::setRouteId(uint32_t routeId)
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
//    smsc_log_debug(smsc::logger::Logger::getInstance("hhh"), "Header: %s, Value: %s", fieldName.c_str(), fieldValue.c_str());
    headerFields[fieldName.c_str()] = fieldValue;
}

void HttpCommand::removeHeaderField(const std::string& fieldName)
{
    headerFields.Delete(fieldName.c_str());
}

HttpRequest::ParameterIterator& HttpRequest::getQueryParameterNames()
{
    queryParameters.First();
    return queryParametersIterator;
}

void HttpRequest::setQueryParameterEncoded(const std::string& paramName,
                                const std::string& _paramValue)
{
    std::string s = _paramValue;
    HttpParser::urlEncode(s);
    queryParameters[paramName.c_str()] = s;
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
        //if(HttpParser::urlDecode(paramValue) != OK)
            //return empty;
        return paramValue;
    }
    else {
        return empty;
    }
}

void HttpRequest::delQueryParameter(const std::string& paramName) {
  queryParameters.Delete(paramName.c_str());
}

void HttpRequest::delHeaderField(const std::string& name) {
    removeHeaderField(name);
    // headerFields.Delete(name.c_str());
}


const std::string& HttpCommand::getMessageText()
{
    const std::string &content_type = getHeaderField(content_type_field);

//    printf("charset=%s content-type=%s\n", charset.c_str(), getHeaderField(content_type_field).c_str());
    if(charset.length() && (textContent.empty() && !strncasecmp(content_type.c_str(), "text/", 5)))
    {
        unsigned int inbytesleft;
        const char *inbufptr = getMessageContent(inbytesleft);

        if(!strcasecmp("UTF-8", charset.c_str()))
            textContent.assign(inbufptr, inbytesleft);
        else
        {
            TmpBuf<uint8_t, 2048> buf(2048);
            Convertor::convert(charset.c_str(), "UTF-8", inbufptr, inbytesleft, buf);
            textContent.assign((char*)buf.get(), buf.GetPos());
        }
    }
        
    return textContent;
}

bool HttpCommand::setMessageText(const std::string& text)
{
    textContent.clear();   

    const std::string &content_type = getHeaderField(content_type_field);

//    printf("charset=%s content-type=%s\n", charset.c_str(), content_type.c_str());
    if(strncasecmp(content_type.c_str(), "text/", 5))
    {
        charset = "UTF-8";
        setHeaderField(content_type_field, "text/html; charset=" + charset);
    }     

    content.SetPos(0);      
    if(!strcasecmp("UTF-8", charset.c_str()))
        content.Append(text.c_str(), text.length());
    else
    {
        TmpBuf<uint8_t, 2048> buf(2048);
        Convertor::convert("UTF-8", charset.c_str(), text.c_str(), text.length(), buf);
        content.Append((char*)buf.get(), buf.GetPos());
        textContent = text;
    }
    textContent = text;        
    setContentLength(content.GetPos());
    setLengthField(content.GetPos());
    removeHeaderField(transfer_encoding);
    removeHeaderField(content_encoding);
    return true;
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
    removeHeaderField(transfer_encoding);
    removeHeaderField(content_encoding);
    
    textContent.clear();
}


HttpRequest::HttpRequest(HttpContext* cx, TransactionContext& tcx) :
HttpCommand(cx, tcx, HTTP_REQUEST),
queryParametersIterator(queryParameters), isInitialRequest(false),
failedBeforeSessionCreate(false)
{}


const std::string& HttpRequest::getUrl()
{
    if ( trc.url.empty() ) {
        char buf[200];
        snprintf(buf,sizeof(buf),"%s:%u",getSite().c_str(),getSitePort());
        std::string headers(buf);
        headers += getSitePath();
        headers += getSiteFileName();
        headers += getURLField();        
        if (queryParameters.GetCount() && httpMethod == GET) {
            headers += '?';
            serializeQuery(headers);
        }
        trc.url.swap(headers);
    }
    return trc.url;
}


void HttpRequest::serializeQuery(std::string& s)
{
    bool n = false;
    char *keystr;
    std::string val;

    queryParameters.First();
    while (queryParameters.Next(keystr, val))
    {
        if(n)
            s += "&";
        else
            n = true;
        s += keystr;
        s += "=";
        HttpParser::urlEncode(val);
        s += val;
    }
}

bool HttpRequest::getPostParams(std::string& params) {
  if (httpMethod == POST) {
    serializeQuery(params);
    return true;
  }
  return false;
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
        headers += getSitePath();
        headers += getSiteFileName();
        headers += getURLField();        

        if (queryParameters.GetCount() && httpMethod == GET) {
            headers += '?';
            serializeQuery(headers);
        }
        
        headers += SP;
        headers += httpVersion;
        headers += CRLF;

        setHeaderField(connection_field, close);
        //removeHeaderField(accept_encoding);   // Was inserted to skip ubsupported encodings
        //removeHeaderField("TE");              // in responses (on message body processing).

        if(httpMethod == POST && !strcasecmp(contentType.c_str(), "application/x-www-form-url-encoded"))
        {
            std::string cnt;
            serializeQuery(cnt);
            content.SetPos(0);
            content.Append(cnt.c_str(), cnt.length());
            setContentLength(cnt.length());
        }
        
        if(cookies.GetCount())
        {
            headers += "Cookie: ";
            defCookie.serialize(headers, false);
            Cookie** ck;
            cookies.First();
            while (cookies.Next(keystr, ck)) {
                (*ck)->serialize(headers, false);
            }
            headers += CRLF;
        }

        setHeaderField("Host", getSite() + ((getSitePort() != 80) ? ':' + std::string(lltostr(getSitePort(), buf + 19)) : ""));

        headerFields.First();
        while (headerFields.Next(keystr, valptr)) {
            headers += keystr;
            headers += ": ";
            headers += *valptr;
            headers += CRLF;
        }
        
        headers += CRLF;
        return headers;
//    }
}


const std::string& HttpResponse::serialize()
{
//    if (headers.empty()) {
        char *keystr;
        std::string *valptr;
        char buf[20];
        buf[19] = 0;
        
        //headers.reserve(totalHeadersSize);
        
        headers = httpVersion;
        headers += SP;

        headers += lltostr(getStatus(), buf + 19);
        headers += SP;
        headers += statusLine;
        headers += CRLF;

        setHeaderField(connection_field, close);
        
        if(cookies.GetCount())
        {
            headers += "Set-Cookie: ";
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
//            smsc_log_debug(smsc::logger::Logger::getInstance("hhh"), "SerializeResponse Header: %s, Value: %s", keystr, valptr);
            headers += keystr;
            headers += ": ";
            headers += *valptr;
            headers += CRLF;
        }
        
        headers += CRLF;
//    }
    return headers;
}

void HttpResponse::fillFakeResponse(int s)
{
    const char *sl;
    size_t len;

    fake = true;
    
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
    case 404:
        sl = "Not Found";
        len = sizeof("Not Found");
        break;
    default:
        sl = "HTTP Transport Error";
        len = sizeof("HTTP Transport Error");
        break;
    }

    setStatus(s);
    statusLine = sl;
    
    if (httpVersion.empty())
        httpVersion = "HTTP/1.1";
            
    headerFields.Assign(StringHash());
    content.SetPos(0);
    content.Append("<html><body><h1>", 16);
    content.Append(sl, len);
    content.Append("</h1></body></html>", 19);
    setLengthField(content.GetPos());
    setHeaderField(content_type_field, "text/html");
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
