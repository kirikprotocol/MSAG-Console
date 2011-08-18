#include "HttpContext.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>

namespace scag2 { namespace transport { namespace http {

using namespace std;
/*
ActionID HttpContext::actionNext[8] = {
    SEND_REQUEST,               // <- PROCESS_REQUEST
    SEND_RESPONSE,              // <- PROCESS_RESPONSE
    NOP,                        // <- PROCESS_STATUS_RESPONSE
    PROCESS_REQUEST,            // <- READ_REQUEST
    READ_RESPONSE,              // <- SEND_REQUEST
    PROCESS_RESPONSE,           // <- READ_RESPONSE
    PROCESS_STATUS_RESPONSE     // <- SEND_RESPONSE
};

const char *HttpContext::taskName[8] = {
    "Scag",             // <- PROCESS_REQUEST
    "Scag",             // <- PROCESS_RESPONSE
    "Scag",             // <- PROCESS_STATUS_RESPONSE
    "Reader",           // <- READ_REQUEST
    "Writer",           // <- SEND_REQUEST
    "Reader",           // <- READ_RESPONSE
    "Writer"            // <- SEND_RESPONSE
};
*/
const char* HttpContext::nameUser = {"user"};
const char* HttpContext::nameSite = {"site"};

unsigned int HttpContext::counter_create;
unsigned int HttpContext::counter_free;

enum StatusCode {
  OK,
  CONTINUE,
  ERROR
};

HttpContext::HttpContext(Socket* userSocket, HttpsOptions* options) :
user(userSocket),
site(NULL),
command(NULL),
action(READ_REQUEST),
requestFailed(false),
unparsed(DFLT_BUF_SIZE),
connectionTimeout(0),
sslOptions(options),
userSsl(NULL),
siteSsl(NULL)
{
	logger = Logger::getInstance("http.https");

	if ( sslOptions ) {
		userHttps = sslOptions->userActive;
		if ( userHttps ) {
			trc.sitePort = 443;
			connectionTimeout = sslOptions->httpsTimeout();
		}
	}
// Mix-protocol: tmp decision
	siteHttps = userHttps;

	setContext(user, this);
}

HttpContext::~HttpContext()
{
	smsc_log_debug(logger, "~HttpContext %p user %p site %p", this, user, site);

	if (user) {
		closeSocketConnection(user, userHttps, userSsl, "User");
		delete user;
	}
	if (site) {
		closeSocketConnection(site, siteHttps, siteSsl, "Site");
		delete site;
	}

	if (command)
		delete command;
}

bool HttpContext::isTimedOut(Socket* s, time_t now) {
	return (now - HttpContext::getTimestamp(s)) >= connectionTimeout;
/* debug block
	long diff = now - HttpContext::getTimestamp(s);
closeSocketConnection(user, userHttps, userSsl, "User") : closeSocketConnection(site, siteHttps, siteSsl, "Site");
	bool rc = diff >= connectionTimeout;
	if (rc)
		smsc_log_debug(logger, "HttpContext::isTimedOut socket %p timeout=%d", s, connectionTimeout);
	return rc;
*/
}

void HttpContext::setSiteHttps(bool supported) {
	siteHttps = supported;
	smsc_log_debug(logger, "HttpContext::setSiteHttps=%s [port=%d]", (siteHttps?"YES":"NO"), command->getSitePort());
}

int HttpContext::sslUserConnection(bool verify_client) {
	smsc_log_debug(logger, "sslUserConnection create.");
	userSsl = SSL_new(sslOptions->userContext());
	if ( userSsl == NULL) {
		smsc_log_debug(logger, "sslUserConnection:create failed.");
		return 0;
	}
	try {
		user->setNonBlocking(0);
		// Assign the socket into the SSL structure (SSL and socket without BIO)
		smsc_log_debug(logger, "sslUserConnection SSL_set_fd.");
		if ( SSL_set_fd(userSsl, user->getSocket()) == 0 )
			throw 0;

		// Perform SSL Handshake on the SSL server
		smsc_log_debug(logger, "sslUserConnection SSL_accept.");
		if ( SSL_accept(userSsl) == -1 ) {
			sslLogErrors();
			throw 0;
		}

		// Get the client's certificate (optional)
//		X509*		client_cert = NULL;
//		client_cert = SSL_get_peer_certificate(userSsl);
//		sslCertInfo(client_cert);

//		smsc_log_debug(logger, "sslUserConnection setNonBlocking."); //getinstance
		user->setNonBlocking(1);
	}
	catch (...) {
		SSL_free(userSsl);
		userSsl = NULL;
		smsc_log_error(logger, "SSL user connection failed");
		return 0;
	}
	HttpContext::counter_create++;
	smsc_log_debug(logger, "sslUserConnection:created %p c:%d f:%d version: %s using %s",
			userSsl, HttpContext::counter_create, HttpContext::counter_free,
			SSL_get_version(userSsl), SSL_get_cipher(userSsl));
	return 1;
}

int HttpContext::sslSiteConnection(bool verify_client) {
	siteSsl = SSL_new(sslOptions->siteContext());
	if ( siteSsl == NULL) {
		smsc_log_debug(logger, "sslSiteConnection:create failed.");
		return 0;
	}
	try {
		// Assign the socket into the SSL structure (SSL and socket without BIO)
		if ( 0 == SSL_set_fd(siteSsl, site->getSocket()) ) {
			smsc_log_error(logger, "sslSiteConnection:Unable to SSL_set_fd set socket.");
			throw 0;
		}

		// Perform SSL Handshake on the SSL client
		site->setNonBlocking(0);
		if ( SSL_connect(siteSsl) <= 0) {
			sslLogErrors();
			throw 0;
		}

		// Get the server's certificate (optional)
//		X509*	server_cert = NULL;
//		server_cert = SSL_get_peer_certificate(siteSsl);
//		sslCertInfo(server_cert);

		site->setNonBlocking(1);
	}
	catch (...) {
		smsc_log_debug(logger, "sslSiteConnection:Exception when configure.");
		SSL_free(siteSsl);
		siteSsl = NULL;
		return 0;
	}
	HttpContext::counter_create++;
	smsc_log_debug(logger, "sslSiteConnection:created %p c:%d f:%d version: %s using %s",
			siteSsl, HttpContext::counter_create, HttpContext::counter_free,
			SSL_get_version(siteSsl), SSL_get_cipher(siteSsl));
	return 1;
}

void HttpContext::closeConnection(Socket* s) {
	smsc_log_debug(logger, "closeConnection: socket %p user %p site %p", s, user, site);
	return (s == user) ? closeSocketConnection(user, userHttps, userSsl, "User") : closeSocketConnection(site, siteHttps, siteSsl, "Site");
}

void HttpContext::closeSocketConnection(Socket* &s, bool httpsFlag, SSL* &ssl, const char* info) {
  try {
	smsc_log_debug(logger, "close%sConnection: socket %p https %s ssl %p", info, s, (httpsFlag?"yes":"no"), ssl);
	if ( httpsFlag ) {
		if ( ssl == NULL ) {
			smsc_log_debug(logger, "close%sConnection: already closed", info);
			return;
		}
		try {
			SSL_shutdown(ssl);
			SSL_free(ssl);
			ssl = NULL;
			HttpContext::counter_free++;
		}
		catch (...) {
			ssl = NULL;
			smsc_log_debug(logger, "close%sConnection: Unknown error c:%d f:%d", info, HttpContext::counter_create, HttpContext::counter_free);
		}
		smsc_log_debug(logger, "close%sConnection: Ok c:%d f:%d", info, HttpContext::counter_create, HttpContext::counter_free);

	}
    if (action != FINALIZE_SOCKET) {
    	s->Abort();
    	delete s;
    	s = NULL;
    }
	smsc_log_debug(logger, "close%sConnection: Ok user:%p site:%p userSsl:%p siteSsl:%p", info, user, site, userSsl, siteSsl);
  }
  catch(...) {
	smsc_log_debug(logger, "close%sConnection: exception user:%p site:%p userSsl:%p siteSsl:%p", info, user, site, userSsl, siteSsl);
  }
}

/*
int HttpContext::sslCloseConnection(Socket* s) {
	if ( !this->useHttps(s) )
		return 0;
	SSL* &ssl = (s == user) ? userSsl : siteSsl;
	if ( ssl == NULL ) {
		smsc_log_debug(logger, "sslCloseConnection: %s already closed", connName(s));
		return 0;
	}
	try {
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = NULL;
		HttpContext::counter_free++;
	}
	catch (...) {
		ssl = NULL;
		smsc_log_debug(logger, "sslCloseConnection: %s Unknown error c:%d f:%d", connName(s), HttpContext::counter_create, HttpContext::counter_free);
	}
	smsc_log_debug(logger, "sslCloseConnection: %s Ok c:%d f:%d", connName(s), HttpContext::counter_create, HttpContext::counter_free);
	return 1;
}
*/

void HttpContext::sslCertInfo(X509* cert) {
	char*	str;
	std::string cert_nfo;
	if (cert != NULL)
	{
		cert_nfo = "Server certificate. Subject: ";

		str = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		if ( str ) {
			cert_nfo.append(str, strlen(str));
			free(str);
		}
		else
			cert_nfo.append("Unknown");

		cert_nfo.append(", Issuer: ");
		str = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		if ( str ) {
			cert_nfo.append(str, strlen(str));
			free(str);
		}
		else
			cert_nfo.append("Unknown");
		cert_nfo.append(".");

		X509_free(cert);
	}
	else
		cert_nfo = "The peer does not have certificate.";
	smsc_log_debug(logger, "%s", cert_nfo.c_str());
}

SSL* HttpContext::sslCheckConnection(Socket* s) {
	if (s == user) {
		if ( NULL == userSsl )
			sslUserConnection();
		return userSsl;
	}
	else if (s == site) {
		if ( NULL == siteSsl )
			sslSiteConnection();
		return siteSsl;
	}
	return NULL;
}

bool HttpContext::useHttps(Socket* s) {
	if (s == user) {
		return ( userHttps );
	}
	else if (s == site) {
		return ( siteHttps );
	}
	return false;
}

int HttpContext::sslReadPartial(Socket* s, const char *readBuf, const size_t readBufSize) {
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL)
		return 0;

	size_t toRead = readBufSize;
	int len=0, err=0, total=0;

/*
 * Messages with length over 16 Kbytes takes more then one TCP package
 * so we have to process several packages to get whole message.
 * After every package read, SSL_pending returns 0. In this case we try to read anyway
 * and finish reading only where SSL_read returns 0.
 */
	do
	{
		smsc_log_debug(logger, "sslReadPartial SSL_read.");
		len = SSL_read(ssl, (void*)readBuf, readBufSize);
		smsc_log_debug(logger, "sslReadPartial: %s cycle, len=%d toRead=%d", connName(s), len, toRead);
		if ( len > 0 ) {
			unparsed.Append(readBuf, len);
			total += len;
			smsc_log_debug(logger, "sslReadPartial: %d chars from %s. Total=%d", len, connName(s), unparsed.GetPos());
			toRead = SSL_pending(ssl);
			if ( toRead >0 )
				continue;
			break;
		}
		else if (len == 0) {
			if ( SSL_get_shutdown(ssl) ) {
				smsc_log_debug(logger, "sslReadPartial: shutdown detected from %s. Total=%d", connName(s), unparsed.GetPos());
				return 0;
			}
/*
 * SSL_pending() returns amount of bytes in SSL receive buffer available to read, but without guarantee.
 * Called before SSL_read(), SSL_pending always returns 0, so we have to call SSL_read first.
 * Called after - it works! (xom 22.06.2011)
 */
			toRead = SSL_pending(ssl);
			if ( toRead > 0 ) {
				smsc_log_debug(logger, "sslReadPartial: not completed from %s. Total=%d, toRead=%d", connName(s), total, toRead);
				continue;
			}
//			s->Abort(); ///
			smsc_log_debug(logger, "sslReadPartial: completed from %s. Total=%d", connName(s), unparsed.GetPos());
			break;  // it seems, the read operation is over here
		}
		else {  //len<0
			err = sslCheckIoError(ssl, len);
/*
			if ( err == CONTINUE ) {
				smsc_log_debug(logger, "sslReadPartial: error from %s. Continue reading.", connName(s));
				continue;
			}
			else
*/
			if ( err == ERROR ) {
				smsc_log_error(logger, "sslReadPartial: Critical error from %s. Exit reading.", connName(s));
				return 0;
			}
			toRead = SSL_pending(ssl);
			if (toRead)
				smsc_log_debug(logger, "sslReadPartial: Managed error from %s. Continue reading toRead=%d", connName(s), toRead);
		}
	} while ( toRead > 0 );
	return total;
}

// non-blocking and partial write allowed: SSL_write returns after every 16kb block
int HttpContext::sslWritePartial(Socket* s, const char* data, const size_t toWrite) {
	int len;
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL) {
		smsc_log_debug(logger, "sslWritePartial: create connection failed");
		return 0;
	}
	while ( toWrite > 0 ) {
		len = SSL_write(ssl, data, toWrite);
		smsc_log_debug(logger, "sslWritePartial: %d of %d chars to %s.", len, toWrite, connName(s));
		if ( SSL_get_shutdown(ssl) ) {
			return 0;
		}
		if ( len > 0 ) {
			return len;
		}
		if (len == 0) {
			break;
//			continue;
		}
		if ( sslCheckIoError(ssl, len) == ERROR ) {
			break;
//			return 0;
		}
	}
	return 0;
}

void HttpContext::beforeReader(void) {
	smsc_log_debug(logger, "HttpContext::beforeReader cx:%p user=%p site=%p", this, user, site);
	unparsed.SetPos(0);
	flags = 0;
	result = 0;
	parsePosition = 0;
}
/*
Socket* HttpContext::beforeWriter(void) {
	smsc_log_debug(logger, "HttpContext::beforeWriter cx:%p user=%p site=%p", this, user, site);
    Socket *s;

    if (action == SEND_REQUEST) {

//TODO: make decision to use HTTPS connection on site (depends on url or route fields);
    	setSiteHttps(command->getSitePort() != 80);
        s = site = new Socket;
        HttpContext::setContext(s, this);
    }
    else
        s = user;

    flags = 0;
    result = 0;
    sendPosition = 0;
    messagePrepare();
    return s;
}
*/

/*
int HttpContext::sendMessage(Socket *s) {
	const char *data;
	unsigned int size;
	int written_size = 0;

	messageGet(s, data, size);
	smsc_log_debug(logger, "data to send %p size=%d pos=%d", data, size, sendPosition);
	if (size) {
		data += sendPosition;
		size -= sendPosition;
		if ( useHttps(s) ) {
			written_size = sslWritePartial(s, data, size);
		}
		else {
			do {
				written_size = s->Write(data, size);
			} while (written_size == -1 && errno == EINTR);
			//	smsc_log_debug(logger, "written %d", written_size);
		}
		smsc_log_error(logger, "actual send %d chars", written_size);
		if (written_size > 0) {
			sendPosition += written_size;
			HttpContext::updateTimestamp(s, time(NULL));
		}
	}
	return written_size;
}
*/

/*
 * Prepare this->command data as suitable (continuous) message buffer to write over HTTPS or chunked.
 * TmpBuf unparsed used to store data, it was unused when SEND_REQUEST or SEND_RESPONSE actions.
 */
void HttpContext::messagePrepare() {
	const char* data;
	unsigned int size, cnt_size;
	sendPosition = 0;
	if (command->chunked) {
		smsc_log_debug(logger, "messagePrepare: chunked flags=%d front=%d chsize=%d", flags, chunks.getData(), chunks.size());
		position = 0;
/*
 		if (flags == 0) {
			position = 0;
		}
		else {
			size = chunks.front();	//	chunks.pop(); on messageIsOver
			std::string ch_size = toString<unsigned int>(size);
			smsc_log_debug(logger, "messagePrepare: chunked flags=%d d:%d=s:%s", flags, size, ch_size.c_str());

			unparsed.SetPos(0);
			unparsed.Append(ch_size.c_str(), ch_size.length());
			unparsed.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
			if ( size ) {
				data = command->content.get();
				data += position;
				unparsed.Append(data, size);
				position += size;
				unparsed.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
			}
			smsc_log_debug(logger, "messagePrepare: chunked flags=1 %s +%d =%d pos=%d", ch_size.c_str(), size, unparsed.GetPos(), position);
		}
*/
		return;
	}

    if ( (siteHttps && (action == SEND_REQUEST)) || (userHttps && (action == SEND_RESPONSE)) ) {
		size_t tmp;
		unparsed.SetPos(0);
		// write headers
		const std::string &headers = command->getMessageHeaders();
		size = headers.size();
		if (size)
			unparsed.Append(headers.data(), size);

		unparsed.Append("\0", 1);
		tmp = unparsed.GetPos();
		if (tmp > 0) {
			unparsed.SetPos(--tmp);
		}
		smsc_log_debug(logger, "messagePrepare: +%d =%d.\n%s", size, unparsed.GetPos(), unparsed.get());
		// write content
		data = command->getMessageContent(cnt_size);
		smsc_log_debug(logger, "messagePrepare h:%d c:%d pos:%d", size, cnt_size, unparsed.GetPos());
		if (cnt_size)
			unparsed.Append(data, cnt_size);
		smsc_log_debug(logger, "messagePrepare: %d + %d = %d.", size, cnt_size, unparsed.GetPos());

		unparsed.Append("\0", 1);
		tmp = unparsed.GetPos();
		if (tmp > 0) {
			unparsed.SetPos(--tmp);
		}
		smsc_log_debug(logger, "messagePrepare: +%d =%d.\n%s", cnt_size, unparsed.GetPos(), unparsed.get());

		flags = 1;
    }
}

/*
 * Analyse if sendPosition reach the end of transmitted data buffer for HTTP and HTTPS
 */
bool HttpContext::messageIsOver(Socket* s) {
	bool rc = false;
	smsc_log_debug(logger, "messageIsOver Https=%s flags=%d sendPosition=%d upos=%d cpos=%d",
			(useHttps(s)?"Yes":"No"), flags, sendPosition, unparsed.GetPos(), command->content.GetPos());
	smsc_log_debug(logger, "messageIsOver chunked=%s position=%d ch_size=%d", ((command->chunked)?"Yes":"No"), position, chunks.size());
	if (command->chunked) {
	    if (flags == 0) {
	    	if ( rc = (sendPosition >= command->getMessageHeaders().size()) ) {
	            flags = 1;
	        }
	    }
	    else {
	    	if ( rc = (sendPosition >= chunks.getSend()) ) {
		    	chunks.remove();
		    	smsc_log_debug(logger, "messageIsOver chunks.remove()= ch_size=%d", chunks.size());
	        }
	    }
	    if (rc) {
	    	prepareNextChunk();
			smsc_log_debug(logger, "prepareNextChunk %s pos=%d send=%d", chunks.getHeader().c_str(), position, chunks.getSend());
	    }
	}
	else {
		if (flags == 0) {
			if ( rc = (sendPosition >= command->getMessageHeaders().size()) ) {
				flags = 1;
				sendPosition = 0;
				rc = (command->content.GetPos() == 0);
			}
		}
		else {
			rc = useHttps(s)
				? (sendPosition >= unparsed.GetPos())
				: (sendPosition >= unsigned(command->content.GetPos()));
		}
	}
	smsc_log_debug(logger, "messageIsOver=%s", (rc?"Yes":"No"));
	return rc;
}

void HttpContext::prepareNextChunk() {
	smsc_log_debug(logger, "prepareNextChunk start ch_size=%d", chunks.size());
	char* tmp;
	unsigned int data_len = 0;
	unparsed.SetPos(0);
	if ( chunks.size() ) {
		unparsed.Append(chunks.getHeader().c_str(), chunks.getHeader().length());
		unparsed.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
		if ( (data_len = chunks.getData()) ) {
			tmp = command->content.get();
			tmp += position;
			unparsed.Append(tmp, data_len);
			unparsed.Append(ChunkInfo::crlf, strlen(ChunkInfo::crlf));
			position += data_len;
		}
		chunks.setSend(unparsed.GetPos());
	}
	sendPosition = 0;
	smsc_log_debug(logger, "prepareNextChunk Ok ch_size=%d", chunks.size());
}

/*
 * HttpContext function to define message (or part of message) attributes: ptr and size
 * that is ready to send
 * both Http and Https
 */
/*
 * returns message ptr and size for HttpWriterTask::Execute depends on useHttps
 */
void HttpContext::messageGet(Socket* s, const char* &data, unsigned int &size) {
	std::string mode;

	if (command->chunked) {
		if (flags == 0) {
			mode = "Chunked hdr";
			// write headers
			data = command->getMessageHeaders().data();
			size = command->getMessageHeaders().size();
		}
		else {
			mode = "Chunked cnt";
			// write next chunk
			data = unparsed.get();
			size = unparsed.GetPos();
		}
	}
	else {
		if ( useHttps(s) ) {
			mode = "Https msg";
			// write whole message headers+content
			flags = 1;
			data = unparsed.get();
			size = unparsed.GetPos();
		}
		else {	// no https
			if (flags == 0) {
				mode = "Http hdr";
				// write headers
				data = command->getMessageHeaders().data();
				size = command->getMessageHeaders().size();
			}
			else {
				mode = "Http cnt";
				// write content
				data = command->getMessageContent(size);
			}
		}
	}
	smsc_log_debug(logger, "messageGet %s %p sz=%d sendPos=%d flags=%d pos=%d", mode.c_str(), data, size, sendPosition, flags, position);
}

/*
 * check SSL io functions ret value in case ret<0
 */
int HttpContext::sslCheckIoError(SSL* ssl, int ret)
{
	int rc = ERROR;
	int oerrno = errno;
	int ssl_err = SSL_get_error(ssl, ret);
	smsc_log_debug(logger, "HttpContext::sslCheckIoError ret:%d errno:%d ssl_err:%d txt:%s\n", ret, ssl_err, oerrno, strerror(oerrno));

	switch ( ssl_err ) {
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		/* apache team preference:
		 * the manual says we have to call SSL_read with the same arguments next time.
		 * we ignore this restriction; no one has complained about it in 1.5 yet, so it probably works anyway.
		 */
		rc = CONTINUE;	// 1-repeat SSL_read with the same params
		break;
	case SSL_ERROR_SYSCALL:
		/**
		 * man SSL_get_error()
		 *
		 * SSL_ERROR_SYSCALL
		 *   Some I/O error occurred.  The OpenSSL error queue may contain more
		 *   information on the error.  If the error queue is empty (i.e.
		 *   ERR_get_error() returns 0), ret can be used to find out more about
		 *   the error: If ret == 0, an EOF was observed that violates the
		 *   protocol.  If ret == -1, the underlying BIO reported an I/O error
		 *   (for socket I/O on Unix systems, consult errno for details).
		 *
		 */
		sslLogErrors();

		smsc_log_debug(logger, "%s %d SSL:%d %d %d %s\n", __FILE__, __LINE__, ret, ssl_err, oerrno, strerror(oerrno));
		break;
	case SSL_ERROR_ZERO_RETURN:
		/* clean shutdown on the remote side */
		smsc_log_error(logger, "SSL_ERROR_ZERO_RETURN:%d %d %d %s\n", ret, ssl_err, oerrno, strerror(oerrno));
		rc = OK;
		break;
//	case SSL_CTRL_SET_TLSEXT_SERVERNAME_ARG:
//		rc = CONTINUE;

	default:
		sslLogErrors();
		break;
	}	//switch
	return rc;
}

void HttpContext::sslLogErrors(void) {
	unsigned long ulerr;
	while ( (ulerr = ERR_get_error()) ) {  /* get all errors from the error-queue */
		smsc_log_error(logger, "%s %d SSL:%d %s\n", __FILE__, __LINE__, ERR_error_string(ulerr, NULL));
	}
}

/************** not used ******************************************
int HttpContext::sslReadMessage(Socket* s, const char *readBuf, const size_t readBufSize) {
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL)
		return 0;

	size_t toRead = readBufSize;
	int len=0;
	size_t total=0;

//
// Messages with length over 16 Kbytes takes more then one TCP package
// so we have to process several packages to get whole message.
// After every package read, SSL_pending returns 0. In this case we try to read anyway
// and finish reading only where SSL_read returns 0.
//
	while ( toRead > 0 ) {
//		smsc_log_debug(logger, "sslReadMessage: %s cycle toRead=%d", connName(s), toRead);
		len = SSL_read(ssl, (void*)readBuf, toRead);
		if (len == 0) {
			if ( SSL_get_shutdown(ssl) ) {
				return 0;
			}
			else
				break;  // it seems, the read operation is over here
			return 0;
		}
		else if ( len > 0 ) {
			smsc_log_debug(logger, "sslReadMessage: %d chars from %s.", len, connName(s));
			unparsed.Append(readBuf, len);
			total = unparsed.GetPos();
			smsc_log_debug(logger, "sslReadMessage: %d chars from %s. Total: %d",
					len, connName(s), total);
//
// SSL_pending() returns amount of bytes in SSL receive buffer available to read, but without guarantee.
// Called before SSL_read(), SSL_pending always returns 0, so we have to call SSL_read first.
// Called after - it works! (xom 22.06.2011)
//
			toRead = SSL_pending(ssl);
			if ( ( 0 == toRead ) || (toRead > readBufSize) )
				toRead = readBufSize;
			continue;
		}
		else {
			toRead = 0;
			if ( sslCheckIoError(ssl, len) == ERROR ) // != CONTINUE )
				break;
		}
	}
//	unparsed.Append("\0", 1);
//	smsc_log_debug(logger, "sslReadMessage:total %d chars from %s:\n%s", total, connName(s), unparsed.get());

	return total;
}
*/
/*
int HttpContext::sslWriteMessage(Socket* s, const char * buf, const size_t buf_size) {
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL) {
		smsc_log_debug(logger, "sslWriteMessage: cant create connection.");
		return 0;
	}

	int toWrite = buf_size;
	int len;
	int total = 0;
	char* data = (char*)buf;
	while ( toWrite > 0 ) {
		smsc_log_debug(logger, "sslWriteMessage: %s cycle toWrite=%d", connName(s), toWrite);
		len = SSL_write(ssl, data, toWrite);
		smsc_log_debug(logger, "sslWriteMessage: %d chars to %s.", len, connName(s));
		if (len == 0) {
			if ( SSL_get_shutdown(ssl) ) {
				return 0;
			}
			continue;
		}
		else if ( len > 0 ) {
			data += len;
			total += len;
			toWrite -= len;
			if ( toWrite <= 0 )
				break;
		}
		else {
			if ( sslCheckIoError(ssl, len) == ERROR ) {
				smsc_log_debug(logger, "sslWriteMessage: sslCheckIoError.");
				return -1;
			}
		}
	}
	return total;
}
*/
/*
int HttpContext::sslWriteCommand(Socket* s) {
	int cnt = 0;
	const char *data;
	unsigned int size;
	unsigned int written_size;
	TmpBuf<char, DFLT_BUF_SIZE> sendBuf;

// write headers
	const std::string &headers = command->getMessageHeaders();
	size = headers.size();
	sendBuf.Append(headers.data(), size);

// write content
	data = command->getMessageContent(written_size);
	sendBuf.Append(data, written_size);
	smsc_log_debug(logger, "sslWriteCommand: hdrs=%d content=%d buf=%d", headers.size(), written_size, sendBuf.GetPos());

	size += written_size;
	data = sendBuf.get();

	cnt = sslWriteMessage(s, data, size);
	if ( cnt <= 0 )
		return 0;  //error
// success: set values that shows whole content have been sent
	flags = 1;
	sendPosition = (unsigned)command->getContentLength();
	return cnt;
}
*/

}}}
