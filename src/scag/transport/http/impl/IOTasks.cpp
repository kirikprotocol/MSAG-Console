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
//#define DEL_REQ         64
//#define DEL_RESP        128
*/

const size_t SHUT_BUF_SIZE = 4;
char SHUT_BUF[SHUT_BUF_SIZE];

void IOTask::killSocket(Socket *s) {
    HttpContext *cx = HttpContext::getContext(s);
    unsigned int flags = cx->flags;
        
	if (!(flags & NO_MULT_REM)) {
        multiplexer.remove(s);
	}

    if ((flags & DEL_SITE_SOCK) && cx->site) {
    	cx->closeConnection(cx->site);
    }
    if ((flags & DEL_USER_SOCK) && cx->user) {
    	cx->closeConnection(cx->user);
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
                multiplexer.remove(s);
//                removeSocket(s);
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

void IOTask::clearSocket(Socket *s) {
    while ((int)recv(s->getSocket(), SHUT_BUF, SHUT_BUF_SIZE, 0) > 0);
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

        if (isTimedOut(s, now)) {
			try {
				HttpContext *cx = HttpContext::getContext(s);
//				smsc_log_debug(logger, "%p: socket %p timeout, cx=%p f:%d a:%s", this, s, cx, cx->flags, cx->actionName());
				if ( cx->isTimedOut(s, now) )
				{
					if (cx->action == SEND_REQUEST)
						cx->setDestiny(408, FAKE_RESP | DEL_SITE_SOCK); //503
					else if (cx->action == SEND_RESPONSE)
						cx->setDestiny(504, STAT_RESP | DEL_SITE_SOCK | DEL_USER_SOCK); //500
					else if (cx->action == READ_REQUEST)
						cx->setDestiny(408, DEL_CONTEXT); //503
					else if (cx->action == READ_RESPONSE)
						cx->setDestiny(504, FAKE_RESP); //503
					else if (cx->action == FINALIZE_SOCKET) {
						smsc_log_warn(logger, "%p::checkConnectionTimeout, socket %p finalization timeout cx=%p", this, s, cx);
//						s->Abort();
						cx->setDestiny(0, STAT_RESP | DEL_SITE_SOCK | DEL_USER_SOCK | DEL_CONTEXT);
					}
					else if (cx->action == KEEP_ALIVE_TIMEOUT) {
//						smsc_log_debug(logger, "%p: socket %p timeout expired, cx=%p f:%d a:%s", this, s, cx, cx->flags, cx->actionName());
//						s->Abort();
						cx->setDestiny(0, STAT_RESP | DEL_SITE_SOCK | DEL_USER_SOCK | DEL_CONTEXT);
					}
					error.Push(s);
				}
			}
			catch (...) {
				smsc_log_debug(logger, "IOTask::checkConnectionTimeout exception cx");
			}
        }
    }
}

int HttpReaderTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    HttpContext *cx;
    char buf[READER_BUF_SIZE];
    unsigned int i;
//    int mx_count;

    smsc_log_debug(logger, "%p started", this);
    
    for (;;) {
//    	smsc_log_debug(logger, "%p for0 m:%d r:%d e:%d, sc:%d iss:%s", this, multiplexer.Count(), ready.Count(), error.Count(), socketCount, (isStopping?"y":"n"));
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

//    	smsc_log_debug(logger, "%p before checkConnectnTimeout m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
        checkConnectionTimeout(error);
//    	smsc_log_debug(logger, "%p before         removeSocket m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
       	removeSocket(error);
//    	smsc_log_debug(logger, "%p, before multiplexer.canRead m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
/*
    	mx_count = multiplexer.canRead(ready, error, SOCKOP_TIMEOUT);
//debug block
    	smsc_log_debug(logger, "%p, after multiplexer.canRead=%d m:%d r:%d e:%d", this, mx_count, multiplexer.Count(), ready.Count(), error.Count());
    	if ( multiplexer.Count() ) {
    		cx = HttpContext::getContext(multiplexer.getSocket(0));
    		smsc_log_debug(logger, "%p, multiplexer socket[0]:%p revents:%x cx:%p", this, multiplexer.getSocket(0), multiplexer.getFds(0)->revents, cx);
    	}
//~debug block
//        if (mx_count > 0) {
*/
        if (multiplexer.canRead(ready, error, SOCKOP_TIMEOUT) > 0) {
            for (i = 0; i < (unsigned int)error.Count(); i++) {
                Socket *s = error[i];           
                cx = HttpContext::getContext(s);
                smsc_log_error(logger, "%p: %p failed (in error array)", this, cx);
                if (cx->action == FINALIZE_SOCKET) {
                    smsc_log_warn(logger, "%p: %p, socket %p finalization error", this, cx, s);
                    cx->setDestiny(0, STAT_RESP | DEL_USER_SOCK);
                	cx->closeConnection(s);
 				    continue;
                }
                if (cx->action == KEEP_ALIVE_TIMEOUT) {
                    smsc_log_warn(logger, "%p: %p, socket %p error where keep-alive timeout", this, cx, s);
                    cx->setDestiny(0, DEL_CONTEXT);
                	cx->closeConnection(s);
 				    continue;
                }

                cx->setDestiny(503, cx->action == READ_RESPONSE ? //503
                    FAKE_RESP | DEL_SITE_SOCK : DEL_CONTEXT);
            }
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
                Socket *s = ready[i];
            	manageReadyRead(s, buf, error);
            }
//        	smsc_log_debug(logger, "%p after manageReadyRead m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
            removeSocket(error);
        }
//    	smsc_log_debug(logger, "%p end of for() m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
    }	// for (;;)

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
//
void HttpReaderTask::manageReadyRead(Socket* s, char* buf, Multiplexer::SockArray &error) {
	HttpContext *cx = HttpContext::getContext(s);
//	smsc_log_debug(logger, "%p: %p, manageReadyRead socket %p", this, cx, s);
    time_t now = time(NULL);

	if (cx->action == KEEP_ALIVE_TIMEOUT) {
		smsc_log_debug(logger, "%p: %p, manageReadyRead socket %p change action from KEEP_ALIVE_TIMEOUT to READ_REQUEST", this, cx, s);
		cx->action = READ_REQUEST;
	}
/*
	if (cx->action == FINALIZE_SOCKET) {
		removeSocket(s);
		clearSocket(s);
		cx->closeConnection(s);
//			smsc_log_debug(logger, "%p: %p, finalize socket %p", this, cx, s);
//			deleteSocket(s, SHUT_WR);
//			smsc_log_debug(logger, "%p: %p, socket %p finalized", this, cx, s);
//			cx->user = NULL;
		cx->action = PROCESS_STATUS_RESPONSE;
		cx->result = 0;
		manager.process(cx);
		return;
	}
*/

	int len;
	bool closed = false;
/*
* To avoid of buffer buf overflow:
* Let read next portion of data into fixed-sized buf, and store in cx->unparsed.
* Then pass cx->unparsed to parser.
*/
	if ( cx->useHttps(s) ) {
		len = cx->sslReadPartial(s, buf, READER_BUF_SIZE, closed);
	}
	else {
		do
			len = s->Read(buf, READER_BUF_SIZE);
		while (len == -1 && errno == EINTR);
		if (len > 0)
			cx->appendUnparsed(buf, len);
		else
			closed = true;
	}
	smsc_log_debug(logger, "%p: %p %p read %d chars, action=%s closed=%s", this, cx, s, len, cx->actionName(), (closed?"y":"n"));

// the case of len=-1 value returned by sslRead... means the peer has closed connection
	if ( len < 0 ) {
		smsc_log_debug(logger, "ssl close connection.");
		removeSocket(s);
		cx->closeConnection(s);
		if ( cx->action == READ_RESPONSE ) {
			cx->setDestiny(405, FAKE_RESP);
			error.Push(s);
		}
		return;
	}
// if keep-alive connection, sslRead... may return 0 after last piece of data, when request/response already parsed. so should be ignored
	if ( !(cx->action == READ_REQUEST || cx->action == READ_RESPONSE) ) {
		smsc_log_debug(logger, "read %d chars, action=%s socket %p remove and continue", len, cx->actionName(), s);
		removeSocket(s);
		return;
	}
//	if (len > 0 || (len == 0 && cx->action == READ_RESPONSE)) {

	switch ( HttpParser::parse(*cx, (len==0)) ) {
	case OK:
		if (cx->action == READ_REQUEST) {
			removeSocket(s);
/*
			if ( cx->useHttps(s) && !closed ) {
				smsc_log_debug(logger, "%p: %p, request parse ok-continue", this, cx);
				break;
			}
*/
			smsc_log_debug(logger, "%p: %p, request parsed, socket %p", this, cx, s);
			cx->action = PROCESS_REQUEST;
		}
		else {	//		if (cx->action == READ_RESPONSE) {
			if ( !closed ) {
				smsc_log_debug(logger, "%p: %p, response parse ok-continue", this, cx);
				break;
			}
			removeSocket(s);
			smsc_log_debug(logger, "%p: %p, response parsed", this, cx);
			if ( cx->command->closeConnection() ) {
				cx->closeConnection(s);
			}
			cx->action = PROCESS_RESPONSE;
		}
		cx->result = 0;
		manager.process(cx);
		break;
	case CONTINUE:
		smsc_log_debug(logger, "%p: %p, parse continue (%s)", this, cx, cx->actionName());
		HttpContext::updateTimestamp(s, now);
		break;
	case ERROR:
	default:
		smsc_log_error(logger, "%p: %p, parse error (%s)", this, cx, cx->actionName());
		cx->setDestiny(405, cx->action == READ_RESPONSE ? (FAKE_RESP | DEL_SITE_SOCK) : FAKE_RESP);
		error.Push(s);
	}
/*
	}
	else {
		smsc_log_error(logger, "%p: %p, read error", this, cx);
		cx->setDestiny(405, cx->action == READ_RESPONSE ?
			(FAKE_RESP | DEL_SITE_SOCK) : DEL_CONTEXT);
		error.Push(s);
	}
*/
}

void HttpReaderTask::registerContext(HttpContext* cx)
{
	cx->beforeReader();
    addSocket(cx->action == READ_RESPONSE ? cx->site : cx->user, true);
}

int HttpWriterTask::Execute()
{
    Multiplexer::SockArray ready;
    Multiplexer::SockArray error;
    unsigned int i;
    HttpContext *cx;

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
            cx = HttpContext::getContext(s);
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
//        	smsc_log_debug(logger, "%p multiplexer.canWrite m:%d r:%d e:%d", this, multiplexer.Count(), ready.Count(), error.Count());
        	if ( multiplexer.Count() ) {
                cx = HttpContext::getContext(multiplexer.getSocket(0));
//            	smsc_log_debug(logger, "%p multiplexer.canWrite socket:%p revents:%x cx:%p", this, multiplexer.getSocket(0), multiplexer.getFds(0)->revents, cx);
        	}
            for (i = 0; i < (unsigned int)error.Count(); i++) {
                Socket *s = error[i];
                cx = HttpContext::getContext(s);
                
                smsc_log_error(logger, "%p: cx:%p socket:%p failed HttpWriterTask action:%s", this, cx, s, cx->actionName());
                if (cx->action == SEND_REQUEST)
                    cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK); 
                else
                    cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);             
            }
            
            for (i = 0; i < (unsigned int)ready.Count(); i++) {
                Socket *s = ready[i];
                manageReadyWrite(s, error);
            }
            removeSocket(error);
        }
    }	// for (;;)

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

void HttpWriterTask::manageReadyWrite(Socket* s, Multiplexer::SockArray &error) {
    HttpContext *cx = HttpContext::getContext(s);
	const char *data;
	unsigned int size;
	int written_size = 0;
    time_t now = time(NULL);

/*
 * HttpContext function to define message (or part of message) attributes: ptr and size
 * that is ready to send depends on https, cx->flags and command->chunked
 * both Http and Https
 */
	cx->messageGet(s, data, size);
	if (size) {
		data += cx->sendPosition;
		size -= cx->sendPosition;
		if ( cx->useHttps(s) ) {
			written_size = cx->sslWritePartial(s, data, size);
		}
		else {
			do {
				written_size = s->Write(data, size);
			} while (written_size == -1 && errno == EINTR);
		}
		smsc_log_error(logger, "%p, actual send %d chars", this, written_size);
		if (written_size > 0) {
			cx->sendPosition += written_size;
			HttpContext::updateTimestamp(s, now);
		}
		else if ( written_size < 0 ) {	// written_size=-1 means the ssl peer has closed connection
			smsc_log_debug(logger, "%p, ssl close connection.", this);
			removeSocket(s);
			cx->closeConnection(s);
			if ( cx->action == SEND_REQUEST ) {
				cx->setDestiny(503, FAKE_RESP);
				error.Push(s);
			}
			return;
		}
		else {
   			smsc_log_error(logger, "%p, manageReadyWrite %p: write error", this, cx);
			if (cx->action == SEND_REQUEST)
				cx->setDestiny(503, FAKE_RESP | DEL_SITE_SOCK);
			else
				cx->setDestiny(500, STAT_RESP | DEL_USER_SOCK);
			error.Push(s);
			return;
        }
	}
/*
 * HttpContext function to analyse
 * if the whole message already sent, depends on cx->flags, cx->sendPosition and message size
 * both Http and Https
 *
 */
    if ( cx->messageIsOver(s) )
    {
//		smsc_log_debug(logger, "HttpWriterTask messageIsOver cx:%p, socket:%p", cx, s);
//if chunked then messageIsOver() returns true on headers and every chunk, so continue
    	if ( cx->command->chunked && cx->chunks.size() ) {
//			smsc_log_debug(logger, "%p: %p, continue with next chunk, final %d chunks to send", this, cx, cx->chunks.size());
   			return;
    	}
//		smsc_log_debug(logger, "%p: %p, removeSocket(%p)", this, cx, s);
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
        	smsc_log_info(logger, "%p: %p, response sent socket %p", this, cx, s);
        	if (cx->command->closeConnection()) {
//				smsc_log_debug(logger, "%p connection: close, cx:%p socket %p", this, cx, s);
//				deleteSocket(s, SHUT_WR);
//				smsc_log_debug(logger, "cx:%p, socket:%p finalized", cx, s);
				cx->action = PROCESS_STATUS_RESPONSE;
				cx->result = 0;
				manager.process(cx);
            }
            else {
//				smsc_log_debug(logger, "%p connection: keep-alive, stay active cx:%p socket:%p", this, cx, s);
            	cx->action = KEEP_ALIVE_TIMEOUT;
				cx->result = 0;
            	manager.readerProcess(cx);
            }
        }
    }
}

void HttpWriterTask::registerContext(HttpContext* cx)
{
//    Socket* s = cx->beforeWriter();
	smsc_log_debug(logger, "HttpWriterTask::registerContext cx:%p user=%p site=%p", cx, cx->user, cx->site);
    Socket *s;

    if (cx->action == SEND_REQUEST) {
/*
 * TODO: make decision to use HTTPS connection on site (depends on url or route fields);
 */
    	if (cx->site)
    		s = cx->site;
    	else {
        	cx->setSiteHttps(cx->command->getSitePort() != 80);
            s = cx->site = new Socket;
            HttpContext::setContext(s, cx);
    	}
    }
    else
        s = cx->user;

    cx->flags = 0;
    cx->result = 0;
    cx->sendPosition = 0;
    cx->messagePrepare();
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
	smsc_log_debug(logger, "%p: addSocket %p.", this, s);
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
