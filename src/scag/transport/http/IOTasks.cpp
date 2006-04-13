#include <errno.h>
#include <time.h>
#include "HttpLogger.h"
#include "IOTasks.h"
#include "HttpContext.h"
#include "Managers.h"
#include "HttpParser.h"

#define READER_BUF_SIZE 8192
#define SOCKOP_TIMEOUT 1000

//extern void printHex(const char *buffer, long length);

namespace scag { namespace transport { namespace http
{

#define NO_MULT_REM     1
#define DEL_SITE_SOCK   2
#define DEL_USER_SOCK   4
#define DEL_CONTEXT     8
#define STAT_RESP       16
#define FAKE_RESP       32
/*
#define DEL_REQ         64
#define DEL_RESP        128
*/

void IOTask::killSocket(Socket *s) {
    HttpContext *cx = HttpContext::getContext(s);
    unsigned int flags = cx->flags;
        
    if (!(flags & NO_MULT_REM))
        multiplexer.remove(s);

    if ((flags & DEL_SITE_SOCK) && cx->site) {
        delete cx->site;
        cx->site = NULL;
    }
    if ((flags & DEL_USER_SOCK) && cx->user) {
        delete cx->user;
        cx->user = NULL;
    }
/*
    if ((flags & (DEL_REQ | DEL_RESP)) && cx->command) {
        delete cx->command;
        cx->command = NULL;
    }
*/
    if (flags & DEL_CONTEXT) {
        delete cx;
    }
    else {
        if (flags & STAT_RESP) {
            cx->action = PROCESS_STATUS_RESPONSE;
            manager.scags.process(cx);  
        }    
        if (flags & FAKE_RESP) {
            cx->action = PROCESS_RESPONSE;
            manager.scags.process(cx);  
        }
    }
}

bool IOTask::isTimedOut(Socket* s, time_t now) {
    return now - HttpContext::getTimestamp(s) >= connectionTimeout;
}

void IOTask::removeSocket(Multiplexer::SockArray &error) {
    if (error.Count()) {
        unsigned int nsub;
        Socket *s;
                
        sockMon.Lock();
        nsub = error.Count();
        while (error.Count()) {
            error.Pop(s);
            killSocket(s);
        }
        sockMon.Unlock();

        iomanager.removeContext(this, nsub);
    }
}

void IOTask::removeSocket(Socket *s) {
    sockMon.Lock();    
    multiplexer.remove(s);
    sockMon.Unlock();
    
    iomanager.removeContext(this, 1);
}

void IOTask::deleteSocket(Socket *s, char *buf, int how) {
    //s->setNonBlocking(0);
    shutdown(s->getSocket(), how);
    while ((int)recv(s->getSocket(), buf, 4, 0) > 0)
        ;                            
    delete s;
}

int HttpReaderTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    char buf[READER_BUF_SIZE];
    unsigned int i;
    time_t now;    

    http_log_debug( "Reader %p started", this );
    
    //http_log_debug( "HttpReaderTask::sockMon == %p", &sockMon );
    
    while (!isStopping) {
        {
            MutexGuard g(sockMon);

            while (!(socketCount || isStopping)) {
                http_log_debug("Reader %p idle", this);
                sockMon.wait();         
                http_log_debug("Reader %p notified", this);
            }
            if (isStopping)
                break;
            
            /* check connection timeout */
            error.Empty();
            now = time(NULL);
            for (i = 0; i < multiplexer.Count(); i++) {
                Socket *s =  multiplexer.getSocket(i);

                if (isTimedOut(s, now)) {
                    HttpContext *cx = HttpContext::getContext(s);
                    
                    http_log_error("Reader %p: %p timed out", this, cx);
                    cx->setDestiny(503, cx->action == READ_RESPONSE ?
                        FAKE_RESP : DEL_CONTEXT);
                    error.Push(s);
                }
            }
        }

        removeSocket(error);

        //http_log_debug("Reader %p: entering the multiplexer");

        if (multiplexer.canRead(ready, error, SOCKOP_TIMEOUT) > 0) {
            for (i = 0; i < error.Count(); i++) {
                Socket *s = error[i];           
                HttpContext *cx = HttpContext::getContext(s);
                
                http_log_error("Reader %p: %p failed", this, cx);
                cx->setDestiny(503, cx->action == READ_RESPONSE ?
                    FAKE_RESP | DEL_SITE_SOCK : DEL_CONTEXT);
            }

            now = time(NULL);
            for (i = 0; i < ready.Count(); i++) {
                Socket *s = ready[i];
                HttpContext *cx = HttpContext::getContext(s);
                unsigned int unparsed_len;
                int len;

                unparsed_len = cx->loadUnparsed(buf);                
                do
                    len = s->Read(buf + unparsed_len, READER_BUF_SIZE - unparsed_len);
                while (len == -1 && errno == EINTR);

                if (len > 0 || (len == 0 && cx->action == READ_RESPONSE)) {
                    http_log_debug("Reader %p: %p, data received", this, cx);
                    unparsed_len += len;
                    len = unparsed_len;

                    //printHex(buf, unparsed_len);
                                                    
                    switch (HttpParser::parse(buf, unparsed_len, *cx)) {
                    case OK:
                        removeSocket(s);
                        if (cx->action == READ_RESPONSE) {
                            http_log_debug("Reader %p: %p, response parsed", this, cx);
                            //cx->setDestiny(0, DEL_SITE_SOCK);
                            deleteSocket(s, buf, SHUT_RD);
                            cx->site = NULL;
                            cx->action = PROCESS_RESPONSE;
                        }
                        else {
                            http_log_debug("Reader %p: %p, request parsed", this, cx);
                            //cx->setDestiny(0, 0);                            
                            cx->action = PROCESS_REQUEST;
                        }
                        cx->result = 0;
                        manager.scags.process(cx);
                        //http_log_debug("Reader %p: finished", this);
                        break;
                    case ERROR:
                        http_log_error("Reader %p: %p, parse error", this, cx);
                        cx->setDestiny(405, cx->action == READ_RESPONSE ?
                            (FAKE_RESP | DEL_SITE_SOCK) : FAKE_RESP);
                        error.Push(s);
                        break;
                    case CONTINUE:
                        HttpContext::updateTimestamp(s, now);
                        cx->saveUnparsed(buf + len - unparsed_len, unparsed_len);
                    }
                }
                else {                    
                    http_log_error("Reader %s: %p, read error", this, cx);
                    cx->setDestiny(405, cx->action == READ_RESPONSE ?
                        (FAKE_RESP | DEL_SITE_SOCK) : DEL_CONTEXT);
                    error.Push(s);
                }
            }

            removeSocket(error);
        }
    }

    {
        Socket *s;
        sockMon.Lock();

        while (multiplexer.Count()) {
            s = multiplexer.getSocket(0);           
            HttpContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
            killSocket(s);
        }
        
        sockMon.Unlock();
    }
    
    http_log_debug( "Reader %p quit", this );
    
    return 0;
}

void HttpReaderTask::registerContext(HttpContext* cx)
{
    Socket *s;

    cx->flags = 0;
    cx->result = 0;
    
    if (cx->action == READ_RESPONSE)
        s = cx->site;   
    else        
        s = cx->user;

    HttpContext::updateTimestamp(s, time(NULL));
    
    addSocket(s);
}

int HttpWriterTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    char buf[4];
    unsigned int i;
    time_t now;

    http_log_debug( "Writer %p started", this );

    //http_log_debug( "HttpWriterTask::sockMon == %p", &sockMon );
    
    while (!isStopping) {
        {
            MutexGuard g(sockMon);

            while (!(socketCount || isStopping)) {
                http_log_debug("Writer %p idle", this);
                sockMon.wait();
                http_log_debug("Writer %p notified", this);
            }
            if (isStopping)
                break;
            
            /* check connection timeout */
            error.Empty();
            now = time(NULL);
            for (i = 0; i < multiplexer.Count(); i++) {
                Socket *s =  multiplexer.getSocket(i);

                if (isTimedOut(s, now)) {
                    HttpContext *cx = HttpContext::getContext(s);
                    
                    http_log_error("Writer %p: %p timed out", this, cx);
                    if (cx->action == SEND_REQUEST)
                        cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
                    else
                        cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);
                    error.Push(s);
                }
            }
            
            /* connect */
            while (waitingConnect.Count()) {
                Socket *s;

                waitingConnect.Pop(s);
                HttpContext *cx = HttpContext::getContext(s);
                http_log_debug("Writer %p: %p, connecting %s:%d", this, cx,
                    cx->getRequest().getSite().c_str(), cx->getRequest().getSitePort());

                if (s->Init(cx->getRequest().getSite().c_str(), cx->getRequest().getSitePort(), SOCKOP_TIMEOUT) ||
                    s->Connect())
                {
                    http_log_error("Writer %p: %p, cannot connect", this, cx);
                    cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK | NO_MULT_REM);
                    error.Push(s);
                }
                else {
                    http_log_debug("Writer %p: %p, socket %p connected", this, cx, s);
                    s->setNonBlocking(1);
                    multiplexer.add(s);
                }
            }
        }

        removeSocket(error);

        //http_log_debug("Reader %p: entering the multiplexer");

        if (multiplexer.canWrite(ready, error, SOCKOP_TIMEOUT) > 0) {
            for (i = 0; i < error.Count(); i++) {
                Socket *s = error[i];
                HttpContext *cx = HttpContext::getContext(s);
                
                http_log_error("Writer %p: %p failed", this, cx);
                if (cx->action == SEND_REQUEST)
                    cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
                else
                    cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);             
            }
            
            now = time(NULL);  
            for (i = 0; i < ready.Count(); i++) {
                Socket *s = ready[i];
                HttpContext *cx = HttpContext::getContext(s);
                const char *data;
                unsigned int size;
                int written_size;
    
                if (cx->flags == 0) {
                    // write headers
                    const std::string &headers = cx->command->getMessageHeaders();

                    data = headers.data() + cx->position;
                    size = headers.size() - cx->position;
                }
                else {
                    // write content
                    data = cx->command->getMessageContent(size) + cx->position;
                    size -= cx->position;
                    
                    //printHex(data, size);
                }
                
                written_size = 0;
                if (size) {
                    /*
                    if (cx->action == SEND_REQUEST)
                        printHex(data, size);
                    */                        
                
                    do
                        written_size = s->Write(data, size);
                    while (written_size == -1 && errno == EINTR);
                    
                    if (written_size > 0) {
                        cx->position += written_size;
                        HttpContext::updateTimestamp(s, now);
                    }
                    else {
                        http_log_error("Writer %p: %p, write error", this, cx);
                        if (cx->action == SEND_REQUEST)
                            cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
                        else
                            cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);
                        error.Push(s);
                    }
                }
                
                if (size == written_size) {
                    if (cx->flags == 0) {                   
                        cx->flags = 1;
                        cx->position = 0;
                    }
                    else {
                        removeSocket(s);
                        if (cx->action == SEND_REQUEST) {
                            http_log_info("Writer %p: %p, request sent", this, cx);
                            //cx->setDestiny(0, DEL_REQ);
                            delete cx->command;
                            cx->command = NULL;
                            cx->action = READ_RESPONSE;
                            cx->result = 0;
                            manager.readers.process(cx);
                        }
                        else {
                            http_log_info("Writer %p: %p, response sent", this, cx);
                            //cx->setDestiny(0, DEL_USER_SOCK);
                            deleteSocket(s, buf, SHUT_WR);
                            cx->user = NULL;
                            cx->action = PROCESS_STATUS_RESPONSE;
                            cx->result = 0;
                            manager.scags.process(cx);
                        }
                        //http_log_debug("Writer %p: finished", this);
                    }
                }
            }
            
            removeSocket(error);
        }
    }

    {
        Socket *s;
        sockMon.Lock();
        
        while (multiplexer.Count()) {
            s = multiplexer.getSocket(0);
            HttpContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
            killSocket(s);
        }

        while (waitingConnect.Count()) {
            waitingConnect.Pop(s);
            HttpContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
            killSocket(s);
        }
        
        sockMon.Unlock();
    }
    
    http_log_debug( "Writer %p quit", this );
    
    return 0;
}

void HttpWriterTask::registerContext(HttpContext* cx)
{
    Socket *s;
    bool connected;

    cx->cleanUnparsed();
    cx->flags = 0;
    cx->result = 0;
    cx->position = 0;    
    
    if (cx->action == SEND_REQUEST) {
        s = cx->site = new Socket;
        HttpContext::setContext(s, cx);         
        connected = false;
    }
    else {
        s = cx->user;
        HttpContext::updateTimestamp(s, time(NULL));
        connected = true;
    }
    
    addSocket(s, connected);
}

IOTask::IOTask(HttpManager& m, IOTaskManager& iom, const int timeout) :
    manager(m), iomanager(iom), connectionTimeout(timeout)
{
}

const char* HttpWriterTask::taskName() {
    return "WriterTask";
}

const char* HttpReaderTask::taskName() {
    return "ReaderTask";
}

void HttpWriterTask::addSocket(Socket* s, bool connected)
{
    sockMon.Lock();
    if (connected)        
        multiplexer.add(s);
    else
        waitingConnect.Push(s);
    sockMon.notify();
    sockMon.Unlock();
}

void HttpReaderTask::addSocket(Socket* s)
{
    sockMon.Lock();    
    multiplexer.add(s);
    sockMon.notify();
    sockMon.Unlock();
}

void IOTask::stop() {
    isStopping = true;
    sockMon.Lock();
    sockMon.notify();    
    sockMon.Unlock();
}

}}}
