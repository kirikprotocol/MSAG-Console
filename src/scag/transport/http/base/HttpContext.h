#ifndef SCAG_TRANSPORT_HTTP_BASE_CONTEXT
#define SCAG_TRANSPORT_HTTP_BASE_CONTEXT

#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "HttpCommand2.h"
#include "logger/Logger.h"
#include "Https.h"
#include "openssl/ssl.h"

#define DFLT_BUF_SIZE 32
#define HTTP_CHUNK_SIZE 0x10000

namespace scag2 { namespace transport { namespace http {

using smsc::core::network::Socket;
using smsc::core::buffers::TmpBuf;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;

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
 * for "site" part - later //TODO
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
    void nextAction() {
        action = actionNext[action];
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
    const char *getTaskName() {
        return taskName[action];
    }

    HttpRequest &getRequest() {
        return *(HttpRequest *)command;
    }
    HttpResponse &getResponse() {
        return *(HttpResponse *)command;
    }

    int sslUserConnection(bool verify_client=false); //server mode connection
    int sslSiteConnection(bool verify_client=false); //client mode connection
    int sslCloseConnection(Socket* s);
    int sslReadPartial(Socket* s, const char* readBuf, const size_t readBufSize);
    int sslWritePartial(Socket* s, const char* data, const size_t toWrite);
//    int sslReadMessage(Socket* s, const char* readBuf, const size_t readBufSize);
//    int sslWriteMessage(Socket* s, const char* buf, const size_t buf_size);
//    int sslWriteCommand(Socket* s);
    bool useHttps(void) { return sslOptions!=NULL; }

    //tmp for debug
    int sslCheckShutdown(void) { return (userSsl) ? SSL_get_shutdown(userSsl) : 0; }
    //for HttpReaderTask::Execute
    char* getUnparsed(void) { return unparsed.get(); }
    unsigned int unparsedLength() { return static_cast<unsigned int>(unparsed.GetPos()); }
    void appendUnparsed(char* buf, unsigned int len) { unparsed.Append(buf, len); }
    void prepareData();
    bool commandIsOver();
    void getCommandAttr(const char* &data, unsigned int &size);

public:
    Socket *user;
    Socket *site;

    HttpCommand *command; // owned

    ActionID action;
    unsigned int flags;
    unsigned int position;
    int result;

    bool requestFailed;

protected:
    enum DataKeys {
        CONTEXT,
        TIMESTAMP,
        CONNECT_FLAG
    };

    static ActionID actionNext[8];
    static const char *taskName[8];
    static const char* nameUser;
    static const char* nameSite;

    TmpBuf<char, DFLT_BUF_SIZE> unparsed;
    TransactionContext trc;
    Logger *logger;

	HttpsOptions* sslOptions;
    SSL*		userSsl;
    SSL*		siteSsl;
    SSL_CTX*	userContext;
    SSL_CTX*	siteContext;
    int sslCheckIoError(SSL* ssl, int ret);
    void sslLogErrors(void);
    SSL* sslCheckConnection(Socket* s);
    void sslCertInfo(X509* cert);
    const char* connName(Socket* s) { return (s==user?nameUser:nameSite); }
};

}}}
#endif // SCAG_TRANSPORT_HTTP_CONTEXT
