#ifndef SCAG_TRANSPORT_HTTP_BASE_COMMAND2
#define SCAG_TRANSPORT_HTTP_BASE_COMMAND2

#include <string>
#include "util/int.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "scag/transport/SCAGCommand2.h"
#include "HttpCommandId.h"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::buffers::TmpBuf;
using smsc::core::buffers::Hash;

class HttpContext;

typedef Hash<std::string> StringHash;

class HttpCommand;
class HttpRequest;
class HttpParser;

const char CONTENT_LENGTH_FIELD[] = "Content-Length";
const char CONTENT_TYPE_FIELD[] = "Content-Type";
const char CONNECTION_FIELD[] = "Connection";
const char KEEP_ALIVE_FIELD[] = "Keep-Alive";
const char TRANSFER_ENCODING_FIELD[] = "Transfer-Encoding";
const char CONTENT_ENCODING_FIELD[] = "Content-Encoding";
const char ACCEPT_ENCODING_FIELD[] = "Accept-Encoding";
const char LATIN_1[] = "ISO8859-1"; //"Latin-1";

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
    std::string url;

    int ruleId;
    // uint16_t usr;

    int result;
    bool statistics;

    TransactionContext() {
        operationId = invalidOpId();
        serviceId = 0;
        providerId = 0;
        ruleId = 0;
        routeId = 0;
        // usr = 0;
        status = 503;
        sitePort = 80;
        result = 0;
        statistics = true;
    }
/*    TransactionContext(TransactionContext& cp) {
        operationId = cp.operationId;
        serviceId = cp.serviceId;
        providerId = cp.providerId;
        ruleId = cp.ruleId;
        routeId = cp.routeId;
        usr = cp.usr;
        status = cp.status;
        sitePort = cp.sitePort;
        site = cp.site;
        sitePath = cp.sitePath;
        siteFileName = cp.siteFileName;
        abonent = cp.abonent;
        address = cp.address;
    }*/
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

// helper for debugging
const char* commandIdName( int );

class Cookie{
public:
    std::string name;
    std::string value;
    StringHash params;

    void serialize(std::string& s, bool set = false);
    const std::string& getParam(const char * name);
    void setParam(const char* nm, const std::string& val);
    void setParam(const char* nm, uint32_t ncnt, const char* val, uint32_t vcnt);
};

typedef Hash<Cookie*> CookieHash;


// NOTE: this one is to nullify session pointer.
class HttpCommandRelease;

/**
 * Abstract class represents generic HttpCommand (Request or Response)
 * Defines basic command methods
 */
class HttpCommand : public SCAGCommand 
{
    friend class HttpParser;
    friend class HttpCommandRelease;

public:
    typedef StringHashIterator FieldIterator;

    HttpCommand(HttpContext* cx, TransactionContext& tcx, uint8_t cmd) :
    headerFieldsIterator(headerFields),
    content(1),
    charset(LATIN_1),
    httpVersion("HTTP/1.1"),
    command_id(cmd),
    session_(0),
    trc(tcx),
    context(cx),
    contentLength(-1), /* totalHeadersSize(0), */
    chunked(false),
    chunk_size(0),
    serial_(makeSerial()),
    closeConnect_(false),
    keepAlive(0)
    {}
    virtual ~HttpCommand();

    // --- scagcommand iface
    virtual TransportType getType() const;
    
    virtual int getServiceId() const;
    virtual void setServiceId(int serviceId);

    // initially SCAGCommand::invalidOpId()
    virtual opid_type getOperationId() const;
    virtual void setOperationId(opid_type op);

    virtual uint8_t getCommandId() const { return command_id; };
    virtual void setCommandId(uint8_t cmd) { command_id = cmd; };

    virtual uint32_t getSerial() const { return serial_; }

    virtual scag2::sessions::Session* getSession() { return session_; }

    virtual void print( util::Print& p ) const;

    virtual const std::string& getUrl() = 0;

    // All values accessible by command context methods
    // should be passed to HttpResponse via HttpRequest 

    // ---------- Command context methods (start) --------
    virtual int getRuleId() const;    // initialy -1
    virtual void setRuleId(int ruleId); 

    virtual uint32_t getRouteId() const;
    virtual void setRouteId(uint32_t routeId);

    virtual int getProviderId() const;
    virtual void setProviderId(int providerId);
    
    virtual bool isResponse() = 0;

    /*
    uint16_t getUSR() {
        return trc.usr;
    }
    void setUSR(uint16_t usr) {
        trc.usr = usr;
    }
     */

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

    void setStatistics(bool statistics) {
      trc.statistics = statistics;
    }
    bool getStatistics() const {
      return trc.statistics;
    }

    // ---------- Command context methods (end) ----------    

    // HTTP message header fields accessors
    FieldIterator& getHeaderFieldNames();
    const std::string& getHeaderField(const std::string& fieldName);
    void setHeaderField(const std::string& fieldName, 
                        const std::string& fieldValue);
                        
    void removeHeaderField(const std::string& fieldName);
    //void serialize();

    HttpContext* getContext() { return context; };

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
        length = static_cast<unsigned int>(content.GetPos());
        return content.get();
    }
    void appendMessageContent(char *data, unsigned int length) {
        content.Append(data, length);
    }
    unsigned int appendChunkedMessageContent(char *data, unsigned int length);

    Cookie* getCookie(const std::string& name);
    Cookie* setCookie(const std::string& name, const std::string& value);
    void delCookie(const std::string& name);
    int getKeepAlive() { return keepAlive; }
    void setKeepAlive(int value) { keepAlive = value; }
    void checkConnectionFields();

protected:
    virtual void setSession( scag2::sessions::Session* s ) { session_ = s; };

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

    scag2::sessions::Session* session_;
        
    TransactionContext &trc;
    HttpContext* context;
    // unsigned int totalHeadersSize;
    int contentLength;
    bool chunked;
    unsigned int chunk_size;
    
    void setLengthField(unsigned int length);
    void setCloseConnection(bool closeConnect) { closeConnect_ = closeConnect; }
    bool closeConnection() const { return closeConnect_; }

private:
    uint32_t  serial_;
    bool closeConnect_;
    int	 keepAlive;
};


class HttpCommandRelease
{
public:
    HttpCommandRelease( HttpCommand& c ) : c_(&c) {}
    ~HttpCommandRelease() { if (c_) c_->setSession(0); }
    void leaveLocked() { c_ = 0; }
private:
    HttpCommandRelease();
    HttpCommandRelease( const HttpCommandRelease& );
    HttpCommandRelease& operator = ( const HttpCommandRelease& );
private:
    HttpCommand* c_;
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

    HttpRequest(HttpContext* cx, TransactionContext& tcx);

    virtual const std::string& getUrl();

    HttpMethod getMethod() {
        return httpMethod;
    }
    void setMethod(HttpMethod m) {
        httpMethod = m;
    }

    const std::string& getSiteQuery() {
        return siteQuery;
    }
    void setSiteQuery(const std::string& p) {
        siteQuery = p;
    }
    
    const std::string& getURLField() {
        return URLField;
    }
    void setURLField(const std::string& p) {
        URLField = p;
    }

    void setInitial() {isInitialRequest = true;};
    bool isInitial() {return isInitialRequest;};

    ParameterIterator& getQueryParameterNames();
    const std::string& getQueryParameter(const std::string& paramName);
    void setQueryParameter(const std::string& paramName, const std::string& _paramValue);
    void setQueryParameterEncoded(const std::string& paramName, const std::string& _paramValue);
    void delQueryParameter(const std::string& paramName);

    // void delCookie(const std::string& name);

    void delHeaderField(const std::string& name);

    const std::string& serialize();
    virtual bool isResponse(); 

    bool isFailedBeforeSessionCreate() const { return failedBeforeSessionCreate; };
    void setFailedBeforeSessionCreate(bool failed) { failedBeforeSessionCreate = failed; };

    bool getPostParams(std::string& params);

protected:
    void serializeQuery(std::string& s);

    static const char *httpMethodNames[9];

    StringHash queryParameters;
    ParameterIterator queryParametersIterator;
    HttpMethod httpMethod;    
    std::string siteQuery;
    std::string siteFull;
    std::string paramValue;
    std::string URLField;    
    
    bool isInitialRequest;
    bool failedBeforeSessionCreate;
};

/**
 * Class provides access to the HTTP response message
 */
class HttpResponse : public HttpCommand {
    friend class HttpParser;

public:
    HttpResponse(HttpContext* cx, TransactionContext& tcx) : HttpCommand(cx, tcx, HTTP_RESPONSE), fake(false) {};

    virtual const std::string& getUrl() {
        return trc.url;
    }

    const std::string& getStatusLine() {
        return statusLine;
    }
    void setStatusLine(const std::string& sl) {
        statusLine = sl;
    }

    bool getDelivered() { return delivered; }
    void setDelivered(bool d) { delivered = d; }
    
    void fillFakeResponse(int s);
    const std::string& serialize();
    virtual bool isResponse();
    bool isFake() const {
      return  fake;
    }
    
protected:
    bool delivered;
    bool fake;
    std::string statusLine;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_COMMAND
