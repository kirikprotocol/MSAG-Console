#include <errno.h>
#include <time.h>
#include "IOTasks.h"
#include "HttpContext.h"
#include "Managers.h"
#include "HttpParser.h"

#define READER_BUF_SIZE 8192
#define SOCKOP_TIMEOUT 1000

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
//        cx->site->Abort();
        delete cx->site;
        cx->site = NULL;
    }
    if ((flags & DEL_USER_SOCK) && cx->user) {
//        cx->user->Abort();
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
            
        {
            Socket *s;      
            MutexGuard g(sockMon);

            nsub = error.Count();
            while (error.Count()) {
                error.Pop(s);
                killSocket(s);
            }
        }

        iomanager.removeContext(this, nsub);
    }
}

void IOTask::removeSocket(Socket *s) {
    {
        MutexGuard g(sockMon);

        multiplexer.remove(s);
    }
    
    iomanager.removeContext(this, 1);
}

void IOTask::deleteSocket(Socket *s, char *buf, int how) {
    //s->setNonBlocking(0);
    shutdown(s->getSocket(), how);
    while ((int)recv(s->getSocket(), buf, 4, 0) > 0)
        ;
//    s->Abort();
    delete s;
}

void IOTask::checkConnectionTimeout(Multiplexer::SockArray& error)
{
    /* check connection timeout */
    error.Empty();
    time_t now = time(NULL);
    for (int i = 0; i < multiplexer.Count(); i++)
    {
        Socket *s =  multiplexer.getSocket(i);

        if (isTimedOut(s, now))
        {
            HttpContext *cx = HttpContext::getContext(s);
                    
            if (cx->action == SEND_REQUEST)
                cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK); //503
            else if (cx->action == SEND_RESPONSE)
                cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK); //500
            else if (cx->action == READ_REQUEST)
                cx->setDestiny(503, DEL_CONTEXT); //503
            else if (cx->action == READ_RESPONSE)
                cx->setDestiny(503, FAKE_RESP); //503
            error.Push(s);
        }
    }
}

int HttpReaderTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    char buf[READER_BUF_SIZE];
    unsigned int i;
    time_t now;    

    smsc_log_debug(logger, "%p started", this);
    
    for (;;) {
        {
            MutexGuard g(sockMon);

            while (!(socketCount || isStopping)) {
                smsc_log_debug(logger, "%p idle", this);
                sockMon.wait();         
                smsc_log_debug(logger, "%p notified", this);
            }
            if (isStopping)
                break;
            
            while(waitingAdd.Count())
            {
                Socket *s;
                waitingAdd.Pop(s);
                multiplexer.add(s);
            }
        }

        checkConnectionTimeout(error);

        removeSocket(error);

        if (multiplexer.canRead(ready, error, SOCKOP_TIMEOUT) > 0) {
            for (i = 0; i < (unsigned int)error.Count(); i++) {
                Socket *s = error[i];           
                HttpContext *cx = HttpContext::getContext(s);
                
                smsc_log_error(logger, "%p: %p failed", this, cx);
                cx->setDestiny(503, cx->action == READ_RESPONSE ? //503
                    FAKE_RESP | DEL_SITE_SOCK : DEL_CONTEXT);
            }
            now = time(NULL);
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
                Socket *s = ready[i];
                HttpContext *cx = HttpContext::getContext(s);
                unsigned int unparsed_len;
                int len;

                unparsed_len = cx->loadUnparsed(buf);                
                do
                    len = s->Read(buf + unparsed_len, READER_BUF_SIZE - unparsed_len);
                while (len == -1 && errno == EINTR);

                if (len > 0 || (len == 0 && cx->action == READ_RESPONSE)) {
                    smsc_log_debug(logger, "%p: %p, data received", this, cx);
                    unparsed_len += len;
                    len = unparsed_len;

                    //printHex(buf, unparsed_len);
                    switch (HttpParser::parse(buf, unparsed_len, *cx)) {
                    case OK:
                        removeSocket(s);
                        if (cx->action == READ_RESPONSE) {
                            smsc_log_debug(logger, "%p: %p, response parsed", this, cx);
                            //cx->setDestiny(0, DEL_SITE_SOCK);
                            s->Abort();
                            delete s;
//                            deleteSocket(s, buf, SHUT_RD);
                            cx->site = NULL;
                            cx->action = PROCESS_RESPONSE;
                        }
                        else {
                            smsc_log_debug(logger, "%p: %p, request parsed", this, cx);
                            //cx->setDestiny(0, 0);
                            cx->action = PROCESS_REQUEST;
                        }
                        cx->result = 0;
                        manager.scags.process(cx);
                        break;
                    case ERROR:
                        smsc_log_error(logger, "%p: %p, parse error", this, cx);
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
                    smsc_log_error(logger, "%p: %p, read error", this, cx);
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
        MutexGuard g(sockMon);

        while (multiplexer.Count()) {
            s = multiplexer.getSocket(0);           
            HttpContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
            killSocket(s);
        }
    }
    
    smsc_log_debug(logger, "%p quit", this);
    
    return 0;
}

void HttpReaderTask::registerContext(HttpContext* cx)
{
    cx->flags = 0;
    cx->result = 0;

    addSocket(cx->action == READ_RESPONSE ? cx->site : cx->user, true);
}

int HttpWriterTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    char buf[4];
    unsigned int i;
    time_t now;

    smsc_log_debug(logger, "%p started");
    
    for (;;) {
        {
            MutexGuard g(sockMon);
            while (!(socketCount || isStopping)) {
                smsc_log_debug(logger, "%p idle", this);
                sockMon.wait();
                smsc_log_debug(logger, "%p notified", this);
            }
            if (isStopping)
                break;
            
            while(waitingAdd.Count())
            {
                Socket *s;
                waitingAdd.Pop(s);
                if (HttpContext::getConnected(s))
                   multiplexer.add(s);
                else
                   waitingConnect.Push(s);
            }

        }

        checkConnectionTimeout(error);

        /* connect */
        while (waitingConnect.Count()) {
            Socket *s;

            waitingConnect.Pop(s);
            HttpContext *cx = HttpContext::getContext(s);
            smsc_log_debug(logger, "%p: %p, connecting %s:%d", this, cx,
            cx->getRequest().getSite().c_str(), cx->getRequest().getSitePort());

            if (s->Init(cx->getRequest().getSite().c_str(), cx->getRequest().getSitePort(), SOCKOP_TIMEOUT) ||
                s->Connect(true))
            {
                smsc_log_error(logger, "%p: %p, cannot connect", this, cx);
                cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK | NO_MULT_REM); //
                error.Push(s);
            }
            else {
                smsc_log_debug(logger, "%p: %p, socket %p connected", this, cx, s);
                //s->setNonBlocking(1);
                multiplexer.add(s);
            }
        }

        removeSocket(error);

        if (multiplexer.canWrite(ready, error, SOCKOP_TIMEOUT) > 0) {
            for (i = 0; i < (unsigned int)error.Count(); i++) {
                Socket *s = error[i];
                HttpContext *cx = HttpContext::getContext(s);
                
                smsc_log_error(logger, "%p: %p failed", this, cx);
                if (cx->action == SEND_REQUEST)
                    cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK); //503
                else
                    cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);              //500
            }
            
            now = time(NULL);  
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
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
                        smsc_log_error(logger, "%p: %p, write error", this, cx);
                        if (cx->action == SEND_REQUEST)
                            cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK); //503
                        else
                            cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK); //500
                        error.Push(s);
                    }
                }
                
                if (size == (unsigned int)written_size) {
                    if (cx->flags == 0) {                   
                        cx->flags = 1;
                        cx->position = 0;
                    }
                    else {
                        removeSocket(s);
                        if (cx->action == SEND_REQUEST) {
                            smsc_log_info(logger, "%p: %p, request sent", this, cx);
                            //cx->setDestiny(0, DEL_REQ);
                            delete cx->command;
                            cx->command = NULL;
                            cx->action = READ_RESPONSE;
                            cx->result = 0;
                            manager.readers.process(cx);
                        }
                        else {
                            smsc_log_info(logger, "%p: %p, response sent", this, cx);
                            //cx->setDestiny(0, DEL_USER_SOCK);
//                            deleteSocket(s, buf, SHUT_WR);
                            delete s;
                            cx->user = NULL;
                            cx->action = PROCESS_STATUS_RESPONSE;
                            cx->result = 0;
                            manager.scags.process(cx);
                        }
                    }
                }
            }
            removeSocket(error);
        }
    }

    {
        Socket *s;
        MutexGuard g(sockMon);
        
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
    }
    
    smsc_log_debug(logger, "%p quit", this);
    
    return 0;
}

void HttpWriterTask::registerContext(HttpContext* cx)
{
    Socket *s;

    cx->cleanUnparsed();
    cx->flags = 0;
    cx->result = 0;
    cx->position = 0;    
    
    if (cx->action == SEND_REQUEST) {
        s = cx->site = new Socket;
        HttpContext::setContext(s, cx);
    }
    else
        s = cx->user;

    addSocket(s, cx->action != SEND_REQUEST);
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

void IOTask::addSocket(Socket* s, bool connected)
{
    MutexGuard g(sockMon);

    HttpContext::updateTimestamp(s, time(NULL));
    HttpContext::setConnected(s, connected);
    waitingAdd.Push(s);

    sockMon.notify();
}

void IOTask::stop() {
    isStopping = true;

    {
        MutexGuard g(sockMon);
    
        sockMon.notify();
    }
}

HttpWriterTask::HttpWriterTask(HttpManager& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout)
{
    logger = Logger::getInstance("scag.http.writer");
}

HttpReaderTask::HttpReaderTask(HttpManager& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout)
{
    logger = Logger::getInstance("scag.http.reader");
}

}}}
