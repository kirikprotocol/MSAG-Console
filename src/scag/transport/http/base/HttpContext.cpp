#include "HttpContext.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>

namespace scag2 { namespace transport { namespace http {

using namespace std;

const char* HttpContext::nameUser = {"User"};
const char* HttpContext::nameSite = {"Site"};
const char* HttpContext::ActionNames[ACTION_LAST] = {
"PROCESS_REQUEST",
"PROCESS_RESPONSE",
"PROCESS_STATUS_RESPONSE",
"PROCESS_LCM",
"FINALIZE_SOCKET",
"READ_REQUEST",
"SEND_REQUEST",
"READ_RESPONSE",
"SEND_RESPONSE",
"KEEP_ALIVE_TIMEOUT"
};

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
	siteHttps = userHttps;

	setContext(user, this);
}

HttpContext::~HttpContext()
{
	try
	{
		if (site) {
			closeSocketConnection(site, siteHttps, siteSsl, nameSite);
			delete site;
		}
		if (user) {
			closeSocketConnection(user, userHttps, userSsl, nameUser);
			delete user;
		}
		if (command)
			delete command;
	}
	catch(...) {
		smsc_log_error(logger, "~HttpContext Exception %p user %p site %p", this, user, site);
	}
}

bool HttpContext::isTimedOut(Socket* s, time_t now) {
	return (now - HttpContext::getTimestamp(s)) >= connectionTimeout;
}

void HttpContext::setSiteHttps(bool supported) {
	siteHttps = supported;
}

int HttpContext::sslUserConnection(bool verify_client) {
	userSsl = SSL_new(sslOptions->userContext());
	if ( userSsl == NULL) {
		smsc_log_error(logger, "sslUserConnection:create failed.");
		return 0;
	}
	try {
		// Assign the socket into the SSL structure (SSL and socket without BIO)
		if ( SSL_set_fd(userSsl, user->getSocket()) == 0 ) {
			smsc_log_error(logger, "sslSiteConnection:Unable to SSL_set_fd set socket.");
			throw 0;
		}

		// Perform SSL Handshake on the SSL server
		user->setNonBlocking(0);
		int ret = 0;
		if ( (ret=SSL_accept(userSsl)) != 1 ) {
			sslLogErrors(ret, SSL_get_error(userSsl, ret));
			throw 0;
		}

		// Get the client's certificate (optional)
//		X509*		client_cert = NULL;
//		client_cert = SSL_get_peer_certificate(userSsl);
//		sslCertInfo(client_cert);

		user->setNonBlocking(1);
	}
	catch (...) {
		SSL_free(userSsl);
		userSsl = NULL;
		smsc_log_error(logger, "SSL user connection failed");
		return 0;
	}
	HttpContext::counter_create++;
	return 1;
}

int HttpContext::sslSiteConnection(bool verify_client) {
	siteSsl = SSL_new(sslOptions->siteContext());
	if ( siteSsl == NULL) {
		smsc_log_error(logger, "sslSiteConnection:create failed.");
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
		int ret = 0;
		if ( (ret=SSL_connect(siteSsl)) != 1) {
			sslLogErrors(ret, SSL_get_error(siteSsl, ret));
			throw 0;
		}

		// Get the server's certificate (optional)
//		X509*	server_cert = NULL;
//		server_cert = SSL_get_peer_certificate(siteSsl);
//		sslCertInfo(server_cert);

		site->setNonBlocking(1);
	}
	catch (...) {
		smsc_log_error(logger, "sslSiteConnection:Exception when configure.");
		SSL_free(siteSsl);
		siteSsl = NULL;
		return 0;
	}
	HttpContext::counter_create++;
	return 1;
}

void HttpContext::closeConnection(Socket* s) {
	return (s == user) ? closeSocketConnection(user, userHttps, userSsl, "User") : closeSocketConnection(site, siteHttps, siteSsl, "Site");
}

void HttpContext::closeSocketConnection(Socket* &s, bool httpsFlag, SSL* &ssl, const char* info) {
	try {
		s->setNonBlocking(0);
		if ( httpsFlag ) {
			if ( ssl == NULL ) {
//				smsc_log_debug(logger, "close%sConnection: already closed", info);
				return;
			}
			try {
				SSL_shutdown(ssl);
				SSL_free(ssl);
				ssl = NULL;
				HttpContext::counter_free++;
			}
			catch(...) {
				ssl = NULL;
				smsc_log_error(logger, "close%sConnection: Unknown error c:%d f:%d", info, HttpContext::counter_create, HttpContext::counter_free);
			}
			smsc_log_debug(logger, "%p close%sConnection: Ok c:%d f:%d", this, info, HttpContext::counter_create, HttpContext::counter_free);

		}
		s->Abort();
		delete s;
		s = NULL;
	}
	catch(...) {
		smsc_log_error(logger, "close%sConnection: exception user:%p site:%p userSsl:%p siteSsl:%p", info, user, site, userSsl, siteSsl);
	}
}

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

int HttpContext::sslReadPartial(Socket* s, const char *readBuf, const size_t readBufSize, bool& closed) {
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL) {
		smsc_log_error(logger, "sslReadPartial: no %s connection return -2", connName(s));
		return -2;
	}

	size_t toRead = readBufSize;
	int len=0, err=0, total=0;
	int ssl_err=0, oerrno=0;
	closed = false;
//
// Messages with length over 16 Kbytes takes more then one TCP package
// so we have to process several packages to get whole message.
// After every package read, SSL_pending returns 0. In this case we try to read anyway
// and finish reading only where SSL_read returns 0.
//
	do
	{
		len = SSL_read(ssl, (void*)readBuf, readBufSize);
		ssl_err = SSL_get_error(ssl, len);
		oerrno = errno;

		if ( len > 0 ) {
			unparsed.Append(readBuf, len);
			total += len;
			if ( (ssl_err == SSL_ERROR_ZERO_RETURN) || (ssl_err == SSL_ERROR_SYSCALL) || SSL_get_shutdown(ssl) )
				closed = true;
			break;
		}
		else if (len == 0) {
			if ( SSL_get_shutdown(ssl) ) {
				smsc_log_debug(logger, "sslReadPartial: shutdown detected from %s. Total=%d, return <total>", connName(s), total);
				closed = true;
				break; //return -1;
			}
//
// SSL_pending() returns amount of bytes in SSL receive buffer available to read, but without guarantee.
// Called before SSL_read(), SSL_pending always returns 0, so we have to call SSL_read first.
// Called after - it works! (xom 22.06.2011)
//
			toRead = SSL_pending(ssl);
			if ( toRead > 0 ) {
				smsc_log_debug(logger, "sslReadPartial: not completed from %s. Total=%d, toRead=%d", connName(s), total, toRead);
				continue;
			}
//			smsc_log_debug(logger, "sslReadPartial: completed from %s. Total=%d", connName(s), unparsed.GetPos());
			closed = true;
			break;  // it seems, the read operation is over here
		}
		else {  //len<0
			err = sslCheckIoError(len, ssl_err, oerrno);
			if ( err == CONTINUE )
				continue;
			if ( err == ERROR ) {
				smsc_log_error(logger, "sslReadPartial: Critical error from %s. Exit reading. return -3", connName(s));
				return -3;
			}
			toRead = SSL_pending(ssl);
			if (toRead)
				smsc_log_error(logger, "sslReadPartial: Managed error from %s. Continue reading toRead=%d", connName(s), toRead);
		}
	} while ( toRead > 0 );
	return total;
}

// non-blocking and partial write allowed: SSL_write returns after every 16kb block
int HttpContext::sslWritePartial(Socket* s, const char* data, const size_t toWrite) {
	int len=0, ssl_err=0,oerrno=0;
	SSL* ssl = sslCheckConnection(s);
	if (ssl == NULL) {
		smsc_log_debug(logger, "sslWritePartial: create connection failed");
		return 0;
	}
	while ( toWrite > 0 ) {
		len = SSL_write(ssl, data, toWrite);
		ssl_err = SSL_get_error(ssl, len);
		oerrno = errno;
		if ( SSL_get_shutdown(ssl) ) {
			return -1;
		}
		if ( len > 0 ) {
			return len;
		}
		if (len == 0) {
			break;
		}
		if ( sslCheckIoError(len, ssl_err, oerrno) == ERROR ) {
			break;
		}
	}
	return 0;
}

void HttpContext::beforeReader(void) {
	unparsed.SetPos(0);
	flags = 0;
	result = 0;
	parsePosition = 0;
}

/*
 * Prepare this->command data as suitable (continuous) message buffer to write over HTTPS or chunked.
 * TmpBuf unparsed used to store data, it was unused when SEND_REQUEST or SEND_RESPONSE actions.
 */
void HttpContext::messagePrepare() {
	const char* data;
	unsigned int size, cnt_size;
	sendPosition = 0;
	if (command->chunked) {
		position = 0;
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
		// write content
		data = command->getMessageContent(cnt_size);
		if (cnt_size)
			unparsed.Append(data, cnt_size);

		unparsed.Append("\0", 1);
		tmp = unparsed.GetPos();
		if (tmp > 0) {
			unparsed.SetPos(--tmp);
		}
		flags = 1;
    }
}

/*
 * Analyse if sendPosition reach the end of transmitted data buffer for HTTP and HTTPS
 */
bool HttpContext::messageIsOver(Socket* s) {
	bool rc = false;
	if (command->chunked) {
	    if (flags == 0) {
	    	if ( rc = (sendPosition >= command->getMessageHeaders().size()) ) {
	            flags = 1;
	        }
	    }
	    else {
	    	if ( rc = (sendPosition >= chunks.getSend()) ) {
		    	chunks.remove();
	        }
	    }
	    if (rc) {
	    	prepareNextChunk();
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
	return rc;
}

void HttpContext::prepareNextChunk() {
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
}

/*
 * check SSL io functions ret value in case ret<0
 */
int HttpContext::sslCheckIoError(int ret, int ssl_err, int oerrno)
{
	int rc = ERROR;

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
		if ( (ret<0) && ( (oerrno==EINTR) || (oerrno==EAGAIN) ) ) {
			rc = CONTINUE;
			break;
		}
		sslLogErrors(ret, ssl_err);
		break;
	case SSL_ERROR_ZERO_RETURN:
		/* clean shutdown on the remote side */
		smsc_log_error(logger, "SSL_ERROR_ZERO_RETURN");
		rc = OK;
		break;
//	case SSL_CTRL_SET_TLSEXT_SERVERNAME_ARG:
//		rc = CONTINUE;

	default:
		sslLogErrors(ret, ssl_err);
		break;
	}
	return rc;
}

void HttpContext::sslLogErrors(int ret, int ssl_err) {
	unsigned long ulerr;
	while ( (ulerr = ERR_get_error()) ) {  /* get all errors from the error-queue */
		smsc_log_error(logger, "SSL:%d %d %d %s", ret, ssl_err, ulerr, ERR_error_string(ulerr, NULL));
	}
}

}}}
