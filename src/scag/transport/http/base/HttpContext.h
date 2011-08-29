#ifndef SCAG_TRANSPORT_HTTP_BASE_CONTEXT
#define SCAG_TRANSPORT_HTTP_BASE_CONTEXT

#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "HttpCommand2.h"
#include "logger/Logger.h"
#include "Https.h"
#include "HttpChunked.h"
#include "openssl/ssl.h"
#include <queue>
#include <sstream>


namespace scag2 { namespace transport { namespace http {

using smsc::core::network::Socket;
using smsc::core::buffers::TmpBuf;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
//using scag2::transport::http::ChunkInfo;

#define DFLT_BUF_SIZE 32

enum ActionID {
    NOP = -1,
    PROCESS_REQUEST = 0,
    PROCESS_RESPONSE,
    PROCESS_STATUS_RESPONSE,
    PROCESS_LCM, 
    FINALIZE_SOCKET,
    READ_REQUEST,
    SEND_REQUEST,
    READ_RESPONSE,
    SEND_RESPONSE
};

/*
 * HTTPS usage is defined
 * for "user" part - on constructor, if HttpsOptions is not NULL
 * for "site" part - see HttpWriterTask::registerContext() :
 * cx->setSiteHttps(cx->command->getSitePort() != 80);
 */

/* no virtual methods are allowed in HttpContext */
class HttpContext 
{
public:
    /* field 'next' must be always the first field */
    HttpContext *next;

    HttpContext(Socket* userSocket, HttpsOptions* options=NULL);
    ~HttpContext();

    unsigned int loadUnparsed(char* buf) {
        unsigned int len = static_cast<unsigned int>(unparsed.GetPos());

        if (len) {
            unparsed.SetPos(0);  
            unparsed.Read(buf, len);
        }

        return len;
    }    
    void saveUnparsed(char* buf, unsigned int len) {
        unparsed.SetPos(0);
        unparsed.Append(buf, len);
    }
    void cleanUnparsed() {
        unparsed.SetPos(0);  
    }
    
    static void setConnected(Socket* s, bool c) {
        s->setData(CONNECT_FLAG, (void*)c);
    }
    static bool getConnected(Socket* s) {
        return (bool)s->getData(CONNECT_FLAG);
    }
    static HttpContext* getContext(Socket* s) {
        return (HttpContext *)s->getData(CONTEXT);
    }
    static void setContext(Socket* s, HttpContext* cx) {
        s->setData(CONTEXT, (void *)cx);
    }
    static void updateTimestamp(Socket* s, time_t t) {
        s->setData(TIMESTAMP, (void *)t);
    }
    static time_t getTimestamp(Socket* s) {
        return (time_t)s->getData(TIMESTAMP);
    }
    void setDestiny(int status, unsigned int flag) {
        result = status;
        this->flags = flag;
    }
    TransactionContext &getTransactionContext() {
        return trc;
    }
    void createFakeResponse(int status) {
        if (!(command && command->isResponse())) {
            if (command)
                delete command;
            command = new HttpResponse(this, trc);
        }
        getResponse().fillFakeResponse(status);
    }
    HttpRequest &getRequest() {
        return *(HttpRequest *)command;
    }
    HttpResponse &getResponse() {
        return *(HttpResponse *)command;
    }
    void setTimeout(int value) {
    	connectionTimeout = value;
    }

    bool isTimedOut(Socket* s, time_t now);

    int sslUserConnection(bool verify_client=false); //server mode connection
    int sslSiteConnection(bool verify_client=false); //client mode connection

    void closeConnection(Socket* s);

    bool useHttps(Socket* s);
    int sslReadPartial(Socket* s, const char* readBuf, const size_t readBufSize);
    int sslWritePartial(Socket* s, const char* data, const size_t toWrite);

    //for HttpParser
    char* getUnparsed(void) { return unparsed.get(); }
    unsigned int unparsedLength() { return static_cast<unsigned int>(unparsed.GetPos()); }
    //for HttpReaderTask
    void beforeReader(void);
    void appendUnparsed(char* buf, unsigned int len) { unparsed.Append(buf, len); }
    //for HttpWriterTask
//    Socket* beforeWriter(void);
    void messagePrepare();
    void messageGet(Socket* s, const char* &data, unsigned int &size);
    bool messageIsOver(Socket* s);
    void setSiteHttps(bool supported); // { siteHttps = supported; }

protected:
    void closeSocketConnection(Socket* &s, bool httpsFlag, SSL* &ssl, const char* info);
    void prepareNextChunk();

public:
    Socket *user;
    Socket *site;

    HttpCommand *command; // owned

    ActionID action;
    unsigned int flags;
    unsigned int position;		// command->content position when send chunked response
    unsigned int parsePosition;	// this->unparsed position when parse
    unsigned int sendPosition;	// this->unparsed position when send message
    int result;

    bool requestFailed;
//temporary for debug log
    static unsigned int counter_create;
    static unsigned int counter_free;
    ChunkInfo chunks;

protected:
    enum DataKeys {
        CONTEXT,
        TIMESTAMP,
//        TIMEOUT,
        CONNECT_FLAG
    };

    static const char* nameUser;
    static const char* nameSite;

//    std::queue<chunk_info> chunks;

    TmpBuf<char, DFLT_BUF_SIZE> unparsed;
    TransactionContext trc;
    Logger *logger;
/*
 * connectionTimeout - performs additional connection check if site response includes "Connection: keep-alive" header
 * default value = cfg:https.timeout;
 * set value when HttpParser::parse found a header "Keep-Alive: value"
 * check value when IOTask::checkConnectionTimeout found default IOTask::connectionTimeout expires
 */
    int connectionTimeout;

	HttpsOptions* sslOptions;
	bool		userHttps; //https usage flags
	bool		siteHttps;
    SSL*		userSsl;
    SSL*		siteSsl;
    int sslCheckIoError(SSL* ssl, int ret);
    void sslLogErrors(void);
    SSL* sslCheckConnection(Socket* s);
    void sslCertInfo(X509* cert);
//temporary for debug log
    const char* connName(Socket* s) { return (s==user?nameUser:nameSite); }

public:
    template<class T>
    static std::string toString(const T& t)
    {
         std::ostringstream stream;
         stream << t;
         return stream.str();
    }
    template<class T>
    static T fromString(const std::string& s)
    {
         std::istringstream stream(s);
         T t;
         stream >> t;
         return t;
    }
};

}}}
#endif // SCAG_TRANSPORT_HTTP_CONTEXT
