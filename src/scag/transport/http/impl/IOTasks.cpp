#include <cerrno>
#include <time.h>
#include "IOTasks.h"
#include "scag/transport/http/base/HttpContext.h"
#include "Managers.h"
#include "scag/transport/http/base/HttpParser.h"

#define READER_BUF_SIZE 8192
#define SOCKOP_TIMEOUT 1000

namespace scag2 {
namespace transport {
namespace http {

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

const size_t SHUT_BUF_SIZE = 4;
char SHUT_BUF[SHUT_BUF_SIZE];

void IOTask::killSocket(Socket *s) {
    HttpContext *cx = HttpContext::getContext(s);
    unsigned int flags = cx->flags;
        
    if (!(flags & NO_MULT_REM))
        multiplexer.remove(s);

    if ((flags & DEL_SITE_SOCK) && cx->site) {
        cx->sslCloseConnection(cx->site);
//        cx->site->Abort();
        delete cx->site;
        cx->site = NULL;
    }
    if ((flags & DEL_USER_SOCK) && cx->user) {
        cx->sslCloseConnection(cx->user);
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
            manager.process(cx);  
        }    
        if (flags & FAKE_RESP) {
            cx->action = PROCESS_RESPONSE;
            if(cx->command == NULL)
                cx->command = new HttpResponse(cx, cx->getTransactionContext());                
            manager.process(cx);  
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

void IOTask::deleteSocket(Socket *s, int how) {
    shutdown(s->getSocket(), how);
    while ((int)recv(s->getSocket(), SHUT_BUF, SHUT_BUF_SIZE, 0) > 0);
    delete s;
}

void IOTask::checkConnectionTimeout(Multiplexer::SockArray& error)
{
    /* check connection timeout */
    error.Empty();
    time_t now = time(NULL);
    for ( size_t i = 0; i < multiplexer.Count(); i++)
    {
        Socket *s =  multiplexer.getSocket(i);

        if (isTimedOut(s, now))
        {
            HttpContext *cx = HttpContext::getContext(s);
            smsc_log_debug(logger, "%p: %p, socket %p timeout, action=%d", this, cx, s, cx->action);

            if (cx->action == SEND_REQUEST)
                cx->setDestiny(408, FAKE_RESP | DEL_SITE_SOCK); //503
            else if (cx->action == SEND_RESPONSE)
                cx->setDestiny(504, STAT_RESP | DEL_USER_SOCK); //500
            else if (cx->action == READ_REQUEST)
                cx->setDestiny(408, DEL_CONTEXT); //503
            else if (cx->action == READ_RESPONSE)
                cx->setDestiny(504, FAKE_RESP); //503
            else if (cx->action == FINALIZE_SOCKET) {
                smsc_log_warn(logger, "%p: %p, socket %p finalization timeout", this, cx, s);
                s->Abort();
                cx->setDestiny(0, STAT_RESP | DEL_USER_SOCK);
            }
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
//        smsc_log_debug(logger, "HttpReaderTask cycle %llu", time(NULL));
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
            smsc_log_debug(logger, "multiplexer.canRead. ready:%d: error:%d", ready.Count(), error.Count());
            for (i = 0; i < (unsigned int)error.Count(); i++) {
                Socket *s = error[i];           
                HttpContext *cx = HttpContext::getContext(s);
                smsc_log_error(logger, "%p: %p failed (in error array)", this, cx);
                if (cx->action == FINALIZE_SOCKET) {
                  cx->sslCloseConnection(s);
                  smsc_log_warn(logger, "%p: %p, socket %p finalization error", this, cx, s);
                  s->Abort();
                  cx->setDestiny(0, STAT_RESP | DEL_USER_SOCK);
                  continue;
                }
                cx->setDestiny(503, cx->action == READ_RESPONSE ? //503
                    FAKE_RESP | DEL_SITE_SOCK : DEL_CONTEXT);
            }
            now = time(NULL);
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
                Socket *s = ready[i];
                HttpContext *cx = HttpContext::getContext(s);
                if (cx->action == FINALIZE_SOCKET) {
                  cx->sslCloseConnection(s);
                  smsc_log_debug(logger, "%p: %p, finalize socket %p", this, cx, s);
                  deleteSocket(s, SHUT_WR);
                  smsc_log_debug(logger, "%p: %p, socket %p finalized", this, cx, s);
                  cx->user = NULL;
                  cx->action = PROCESS_STATUS_RESPONSE;
                  cx->result = 0;
                  manager.process(cx);
                  removeSocket(s);
                  continue;
                }

                int len;
/*
                unsigned int unparsed_len;
                if ( cx->useHttps(s) ) {
                	len = cx->sslReadMessage(s, buf, READER_BUF_SIZE);
                    unparsed_len = cx->loadUnparsed(buf);
                    unparsed_len = 0;
                    smsc_log_debug(logger, "HTTPS data received. len=%d", len);
                }
                else {
                    unparsed_len = cx->loadUnparsed(buf);
					do
						len = s->Read(buf + unparsed_len, READER_BUF_SIZE - unparsed_len);
					while (len == -1 && errno == EINTR);
                }
                if (len > 0 || (len == 0 && cx->action == READ_RESPONSE)) {
//                    smsc_log_debug(logger, "%p: %p, data received. len=%d", this, cx, len);
                    unparsed_len += len;
                    len = unparsed_len;
*/
                /*
                 * to avoid of buffer buf overflow
                 */
                smsc_log_debug(logger, "read from socket %p to context %p", s, cx);
                if ( cx->useHttps(s) ) {
//                	len = cx->sslReadMessage(s, buf, READER_BUF_SIZE);
                	len = cx->sslReadPartial(s, buf, READER_BUF_SIZE);
                }
                else {
                	do
 						len = s->Read(buf, READER_BUF_SIZE);
 					while (len == -1 && errno == EINTR);
                    if (len > 0) {
                    	cx->appendUnparsed(buf, len);
                    }
                 }
                smsc_log_debug(logger, "read %d chars, action=%d", len, cx->action);
                if (len > 0 || (len == 0 && cx->action == READ_RESPONSE)) {
                	if ( cx->command == NULL )
                		smsc_log_debug(logger, "parse start. f:%d pp:%d h:00 c:00 w:00", cx->flags, cx->parsePosition);
                	else {
                   		smsc_log_debug(logger, "parse start. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
                			cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
                			cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
                	}
                	StatusCode sc = HttpParser::parse(*cx);
            		smsc_log_debug(logger, "parse result %d", sc);
            		switch ( sc ) {
                    case OK:
                		smsc_log_debug(logger, "parse OK. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
                			cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
                			cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
                        removeSocket(s);
                        if (cx->action == READ_RESPONSE) {
                            smsc_log_debug(logger, "%p: %p, response parsed", this, cx);
                            if ( cx->useHttps(s) ) {
                            	cx->sslCloseConnection(s);
                            }
                            s->Abort();
                            delete s;
                            cx->site = NULL;
                            cx->action = PROCESS_RESPONSE;
                        }
                        else {
                            smsc_log_debug(logger, "%p: %p, request parsed", this, cx);
                            cx->action = PROCESS_REQUEST;
                        }
                        cx->result = 0;
                        manager.process(cx);
                        break;
                    case CONTINUE:
                		smsc_log_debug(logger, "parse CONTINUE. f:%d pp:%d h:%d c:%d w:%d d:%p sz:%d", cx->flags, cx->parsePosition,
                			cx->command->getMessageHeaders().size(), cx->command->getContentLength(),
                			cx->command->content.GetPos(), cx->command->content.get(), cx->command->content.getSize());
                    	HttpContext::updateTimestamp(s, now);
//                        cx->saveUnparsed(buf + len - unparsed_len, unparsed_len);
                        break;
                    case ERROR:
                    default:
                    	smsc_log_debug(logger, "parse ERROR=%d", sc);
                        smsc_log_error(logger, "%p: %p, parse error", this, cx);
                        cx->setDestiny(405, cx->action == READ_RESPONSE ?
                            (FAKE_RESP | DEL_SITE_SOCK) : FAKE_RESP);
                        error.Push(s);
                        break;
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
    cx->cleanUnparsed();
    cx->flags = 0;
    cx->result = 0;
    cx->position = 0;
    cx->parsePosition = 0;

    addSocket(cx->action == READ_RESPONSE ? cx->site : cx->user, true);
}

int HttpWriterTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    unsigned int i;
    time_t now;

    smsc_log_debug(logger, "%p started",this);
    
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
                    cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK); 
                else
                    cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);             
            }
            
            now = time(NULL);  
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
                Socket *s = ready[i];
                HttpContext *cx = HttpContext::getContext(s);
                const char *data;
                unsigned int size;
                int written_size = 0;

/*
            	if ( cx->useHttps(s) ) {
					cx->flags = 1;
					data = cx->getUnparsed() + cx->position;
					size = cx->unparsedLength() - cx->position;
					written_size = cx->sslWritePartial(s, data, size);
//					written_size = cx->sslWriteCommand(s);
					if (written_size > 0) {
						cx->position += written_size;
						HttpContext::updateTimestamp(s, now);
					}
					else {
						if (cx->action == SEND_REQUEST)
							cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
						else
							cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);
						error.Push(s);
						continue;
					}
            	}
            	else {	// no https
					if (cx->flags == 0) {
						// write headers
						const std::string &headers = cx->command->getMessageHeaders();

						data = headers.data() + cx->position;
						size = headers.size() - cx->position;
					}
					else {
						// write content
						data = cx->command->getMessageContent(size);
						data += cx->position;
						size -= cx->position;
					}

*/
                cx->messageGet(s, data, size);
				smsc_log_debug(logger, "data to send %p size=%d pos=%d", data, size, cx->position);
				if (size) {
					data += cx->position;
					size -= cx->position;
					if ( cx->useHttps(s) ) {
						written_size = cx->sslWritePartial(s, data, size);
					}
					else {
						do {
							written_size = s->Write(data, size);
						} while (written_size == -1 && errno == EINTR);
						smsc_log_debug(logger, "written %d", written_size);
					}
					smsc_log_error(logger, "actual send %d chars", written_size);
					if (written_size > 0) {
						cx->position += written_size;
						HttpContext::updateTimestamp(s, now);
					}
					else {
						smsc_log_error(logger, "%p: %p, write error", this, cx);
						if (cx->action == SEND_REQUEST)
							cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
						else
							cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);
						error.Push(s);
						continue;
					}
				}
/*
                if (cx->flags == 0)
                {
                    if(cx->position >= cx->command->getMessageHeaders().size())
                    {
                        cx->flags = 1;
                        cx->position = 0;
                    }
                }
                else
*/
                if ( cx->messageIsOver(s) )
                {
                    removeSocket(s);
                    if (cx->action == SEND_REQUEST) {
                        smsc_log_info(logger, "%p: %p, request sent", this, cx);
                        delete cx->command;
                        cx->command = NULL;
                        cx->action = READ_RESPONSE;
                        cx->result = 0;
                        manager.readerProcess(cx);
                    }
                    else {
                        smsc_log_info(logger, "%p: %p, response sent", this, cx);
                        if (cx->command->closeConnection()) {
                          smsc_log_debug(logger, "%p: %p, close connection, finalize socket %p", this, cx, s);
                          if ( cx->useHttps(s) )
                        	  cx->sslCloseConnection(s);
                          deleteSocket(s, SHUT_WR);
                          smsc_log_debug(logger, "%p: %p, socket %p finalized", this, cx, s);
                          cx->user = NULL;
                          cx->action = PROCESS_STATUS_RESPONSE;
                          cx->result = 0;
                          manager.process(cx);
                        } else {
                          cx->action = FINALIZE_SOCKET;
                          manager.readerProcess(cx);
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
/*
 * TODO: make decision to use HTTPS connection on site (depends on url or route fields);
 */
    	cx->setSiteHttps(cx->command->getSitePort() == 443);
        s = cx->site = new Socket;
        HttpContext::setContext(s, cx);
    }
    else
        s = cx->user;

    //debug block
    {
    	unsigned int buf_len;
		const char* data = cx->command->getMessageContent(buf_len);
    	smsc_log_debug(logger, "HttpWriterTask::registerContext. h:%d c:%d w:%d d:%p",
    			cx->command->getMessageHeaders().size(), cx->command->getContentLength(), buf_len, data);
    }
    cx->messagePrepare(); //xom 14.07.2011
    addSocket(s, cx->action != SEND_REQUEST);
}

IOTask::IOTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout) :
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

HttpWriterTask::HttpWriterTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout)
{
    logger = Logger::getInstance("http.writer");
}

HttpReaderTask::HttpReaderTask(HttpManagerImpl& m, IOTaskManager& iom, const int timeout) :
        IOTask(m, iom, timeout)
{
    logger = Logger::getInstance("http.reader");
}

}}}
