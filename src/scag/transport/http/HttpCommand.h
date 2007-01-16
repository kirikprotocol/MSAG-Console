#ifndef SCAG_TRANSPORT_HTTP_COMMAND
#define SCAG_TRANSPORT_HTTP_COMMAND

#include <string>
#include "util/int.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::buffers::TmpBuf;
using smsc::core::buffers::Hash;
using scag::transport::SCAGCommand;

typedef Hash<std::string> StringHash;

const char CONTENT_LENGTH_FIELD[] = "content-length";
const char CONTENT_TYPE_FIELD[] = "content-type";
const char CONNECTION_FIELD[] = "connection";
const char LATIN_1[] = "ISO8859-1"; //"Latin-1";

class HttpCommand;
class HttpRequest;
class HttpParser;

struct TransactionContext {
    int64_t operationId;
    uint32_t serviceId;
    uint32_t providerId;
    uint32_t routeId;
    std::string abonent;
    std::string address;
    int status;    
    unsigned int sitePort;
    std::string site;
    std::string sitePath;    
    std::string siteFileName;
    
    int ruleId;
    uint16_t usr;

    TransactionContext() {
        operationId = -1;
        serviceId = 0;
        providerId = 0;
        ruleId = 0;
        routeId = 0;
        usr = 0;
        status = 0;
        sitePort = 80;
    }
};

/**
 * Service class to iterate field names
 */
class StringHashIterator
{
public:
    StringHashIterator(StringHash &hash) : masterHash(hash) {}

    bool next();
    const std::string& value() {
        return key;
    }

protected:
    std::string key;
    StringHash &masterHash;
};

enum HttpCommandId {
    HTTP_REQUEST = 100,
    HTTP_RESPONSE,
    HTTP_DELIVERY
};

class Cookie{
public:
    std::string name;
    std::string value;
    StringHash params;

    void serialize(std::string& s, bool set = false);
    const std::string& getParam(const char * name);
    void setParam(const char* nm, std::string& val);
    void setParam(const char* nm, uint32_t ncnt, const char* val, uint32_t vcnt);
};

typedef Hash<Cookie*> CookieHash;

/**
 * Abstract class represents generic HttpCommand (Request or Response)
 * Defines basic command methods
 */
class HttpCommand : public SCAGCommand {
    friend class HttpParser;

    LongCallContext lcmCtx;
    
public:
    typedef StringHashIterator FieldIterator;

    HttpCommand(TransactionContext& tcx, uint8_t cmd) : trc(tcx), content(1), command_id(cmd),
        contentLength(-1), /* totalHeadersSize(0), */
        headerFieldsIterator(headerFields), charset(LATIN_1) {}
    virtual ~HttpCommand();

    virtual LongCallContext& getLongCallContext()
    {
        return lcmCtx;            
    }
    
    virtual TransportType getType() const;
    
    // All values accessible by command context methods
    // should be passed to HttpResponse via HttpRequest 

    // ---------- Command context methods (start) --------
    virtual int getRuleId() const;    // initialy -1
    virtual void setRuleId(int ruleId); 

    virtual uint32_t getRouteId() const;
    virtual void setRouteId(uint32_t routeId);

    virtual int getServiceId() const;
    virtual void setServiceId(int serviceId);

    virtual int getProviderId() const;
    virtual void setProviderId(int providerId);
    
    virtual int64_t getOperationId() const;
    virtual void setOperationId(int64_t op);   // initialy -1

    virtual bool isResponse() = 0;
    virtual uint8_t getCommandId() const { return command_id; };
    virtual void setCommandId(uint8_t cmd) { command_id = cmd; };

    uint16_t getUSR() {
        return trc.usr;
    }
    void setUSR(uint16_t usr) {
        trc.usr = usr;
    }

    const std::string& getAbonent() {
        return trc.abonent;
    }
    void setAbonent(const std::string& abonent) {
        trc.abonent = abonent;
    }
    const std::string& getAddress() {
        return trc.address;
    }
    void setAddress(const std::string& address) {
        trc.address = address;
    }
    
    const std::string& getSitePath() {
        return trc.sitePath;
    }
    void setSitePath(const std::string& path) {
        trc.sitePath = path;
    }    
    
    const std::string& getSiteFileName() {
        return trc.siteFileName;
    }
    void setSiteFileName(const std::string& fn) {
        trc.siteFileName = fn;
    }
    
    const std::string& getSite() {
        return trc.site;
    }
    void setSite(const std::string& s) {
        trc.site = s;
    }    
    
    unsigned int getSitePort() {    
        return trc.sitePort;
    }
    void setSitePort(unsigned int port) {    
        trc.sitePort = port;
    }
    
    int getStatus() {
        return trc.status;
    }
    void setStatus(int s) {
        trc.status = s;
    }
    
    // ---------- Command context methods (end) ----------    

    // HTTP message header fields accessors
    FieldIterator& getHeaderFieldNames();
    const std::string& getHeaderField(const std::string& fieldName);
    void setHeaderField(const std::string& fieldName, 
                        const std::string& fieldValue);

    //void serialize();

    const std::string& getMessageHeaders() {
        return headers;
    }

    int getContentLength() {
        return contentLength < 0 ? 0 : contentLength;
    }
    void setContentLength(int length) { 
        contentLength = length;
        content.setSize(length);
    }
    
    const std::string& getMessageText();
    bool setMessageText(const std::string& text);

    const uint8_t* getMessageBinary(int& length);
    void setMessageBinary(uint8_t* body, int length, const std::string& cp);

    char *getMessageContent(unsigned int& length) {
        length = content.GetPos();
        return content.get();
    }
    void appendMessageContent(char *data, unsigned int length) {
        content.Append(data, length);
    }

    Cookie* getCookie(const std::string& name);
    Cookie* setCookie(const std::string& name, const std::string& value);

public:
    StringHash headerFields;
    FieldIterator headerFieldsIterator;
    TmpBuf<char, 1> content;
    std::string headers;
    std::string charset;
    std::string contentType;
    std::string httpVersion;
    std::string textContent;
    Cookie  defCookie;
    CookieHash cookies;
    uint8_t command_id;
        
    TransactionContext &trc;
    // unsigned int totalHeadersSize;
    int contentLength;
    
    void setLengthField(unsigned int length);
};

/**
* HTTP Request methods
*/ 
enum HttpMethod {
    GET = 1,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
    OPTIONS
};

/**
 * Class provides access to the HTTP request message
 */
class HttpRequest : public HttpCommand {
    friend class HttpParser;

public:
    typedef StringHashIterator ParameterIterator;

    HttpRequest(TransactionContext& tcx) : HttpCommand(tcx, HTTP_REQUEST),
        queryParametersIterator(queryParameters), isInitialRequest(false) {}

    HttpMethod getMethod() {
        return httpMethod;
    }
    void setMethod(HttpMethod m) {
        httpMethod = m;
    }

    const std::string& getSiteQuery() {
        return siteQuery;
    }
    void setSiteQuery(const std::string& query) {
        siteQuery = query;
    }

    void setInitial() {isInitialRequest = true;};
    bool isInitial() {return isInitialRequest;};

    ParameterIterator& getQueryParameterNames();
    const std::string& getQueryParameter(const std::string& paramName);
    void setQueryParameter(const std::string& paramName, const std::string& _paramValue);
    void setQueryParameterEncoded(const std::string& paramName, const std::string& _paramValue);

    const std::string& serialize();
    virtual bool isResponse();
protected:
    void serializeQuery(std::string& s);

    static const char *httpMethodNames[9];

    StringHash queryParameters;
    ParameterIterator queryParametersIterator;
    HttpMethod httpMethod;    
    std::string siteQuery;
    std::string siteFull;
    std::string paramValue;
    
    bool isInitialRequest;
};

/**
 * Class provides access to the HTTP response message
 */
class HttpResponse : public HttpCommand {
    friend class HttpParser;

public:
    HttpResponse(TransactionContext& tcx) : HttpCommand(tcx, HTTP_RESPONSE) {};

    const std::string& getStatusLine() {
        return statusLine;
    }
    void setStatusLine(const std::string& sl) {
        statusLine = sl;
    }

    bool getDelivered() { return delivered; }
    void setDelivered(bool d) { delivered = d; }
    
    void fillFakeResponse(int s);
    void serialize();
    virtual bool isResponse();
    
protected:
    bool delivered;
    std::string statusLine;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_COMMAND
