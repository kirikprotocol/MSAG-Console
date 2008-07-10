#ifndef SCAG_TRANSPORT_HTTP_CONTEXT
#define SCAG_TRANSPORT_HTTP_CONTEXT

#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "HttpCommand.h"

#define DFLT_BUF_SIZE 32

namespace scag { namespace transport { namespace http
{
using smsc::core::network::Socket;
using smsc::core::buffers::TmpBuf;
using smsc::core::synchronization::Mutex;

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

/* no virtual methods are allowed in HttpContext */
class HttpContext {
public:
    /* field 'next' must be always the first field */
    HttpContext *next;

    HttpContext(Socket* userSocket) : action(READ_REQUEST),
        user(userSocket), site(NULL), command(NULL),
        unparsed(DFLT_BUF_SIZE), requestFailed(false)
    {
        setContext(user, this); 
    }

    ~HttpContext();

    unsigned int loadUnparsed(char* buf) {
        unsigned int len = unparsed.GetPos();

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
    
    TransactionContext &getTransactionContext() {
        return trc;
    }

    void setDestiny(int status, unsigned int flag) {
        result = status;
        this->flags = flag;
    }
    void nextAction() {
        action = actionNext[action];
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

    Socket *user;
    Socket *site;

    HttpCommand *command;

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

    TmpBuf<char, DFLT_BUF_SIZE> unparsed;
    TransactionContext trc;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_CONTEXT
