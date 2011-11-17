/* OpenSSL options and threads support.
 * First, today we are using OpenSSL 0.9.8
 *
 * Warning:
 * In openssl version 1.0.0 these (thread id) functions were replaced with CRYPTO_THREADID_...
 * CRYPTO_THREADID_set_callback((unsigned long (*)())thread_id_function);
 * Todo: add openssl version 1.0.0 support here.
 *
 * Second
 * Also, dynamic locks are currently not used internally by OpenSSL, but
 * may do so in the future.
 *
 */

#include "Https.h"
#include <openssl/err.h>

namespace scag2 { namespace transport { namespace http {

#define OPENSSL_THREAD_DEFINES
#include <openssl/opensslconf.h>
#if defined(OPENSSL_THREADS)
const char* HttpsThreads::diagnostics = "OpenSSL: thread support enabled";
#else
const char* HttpsThreads::diagnostics = "OpenSSL: no thread support";
#endif

HttpsThreads* HttpsOptions::HttpsThreadsSupport = NULL;
pthread_mutex_t* HttpsThreads::mutex_buf = NULL;
long* HttpsThreads::lock_count = NULL;

// HttpOptions: relative to cfg.httpsCertificates
#define RSA_SERVER_CERT     "/s_cert.pem"
#define RSA_SERVER_KEY      "/s_privkey.pem"

#define RSA_SERVER_CA_CERT	"/server_ca.crt"
#define RSA_SERVER_CA_PATH	"/CA"

#define RSA_CLIENT_CERT		"/c_cert.pem"
#define RSA_CLIENT_KEY		"/c_privkey.pem"

#define RSA_CLIENT_CA_CERT  "/client_ca.crt"
#define RSA_CLIENT_CA_PATH  "/CA"


int HttpsThreads::setup() {
	smsc_log_debug(smsc::logger::Logger::getInstance("http.threads"), "%p setup. CRYPTO_num_locks=%d", this, CRYPTO_num_locks());
	mutex_buf = (pthread_mutex_t*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	lock_count = (long*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	if (!mutex_buf)
		return 0;
	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		lock_count[i]=0;
		pthread_mutex_init(&mutex_buf[i], NULL);
	}

	CRYPTO_set_id_callback(/*(unsigned long (*)())*/thread_id_function);
	CRYPTO_set_locking_callback(/*(void (*)())*/locking_function);
	return 1;
}
int HttpsThreads::cleanup() {
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);

	if (!mutex_buf)
		return 0;

	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_destroy(&mutex_buf[i]);
		if (lock_count[i])
			smsc_log_debug(smsc::logger::Logger::getInstance("http.threads"), "lock[%d]:%d:%s", i, lock_count[i], CRYPTO_get_lock_name(i));
	}

	OPENSSL_free(mutex_buf);
	mutex_buf = NULL;
	OPENSSL_free(lock_count);
	lock_count = NULL;

	return 1;
}

void HttpsThreads::locking_function(int mode, int type, const char *file, int line) {
//	smsc_log_debug(smsc::logger::Logger::getInstance("http.threads"), "lock thread:%u %s:%s mode:%d type:%d %s:%d", CRYPTO_thread_id(),
//			(mode&CRYPTO_LOCK)?"l":"u",	(type&CRYPTO_READ)?"r":"w", mode, type, file, line);

	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&mutex_buf[type]);
		lock_count[type]++;
	}
	else {
		pthread_mutex_unlock(&mutex_buf[type]);
	}
}
//-----------------------------------------------------------------------------------------------------
HttpsOptions::~HttpsOptions()
{
	if ( siteCtx ) {
		SSL_CTX_free(siteCtx);
		siteCtx = NULL;
	}
	if ( userCtx ) {
		SSL_CTX_free(userCtx);
		userCtx = NULL;
	}
	if ( HttpsThreadsSupport ) {
		delete HttpsThreadsSupport;
		HttpsThreadsSupport = NULL;
	}
}

int HttpsOptions::init(bool user_verify, bool site_verify, const config::HttpManagerConfig* conf) {
	this->cfg = conf;
	int result=0;
	logger = Logger::getInstance("http.https");

	userActive = false;
	siteActive = false;
	if ( cfg->httpsEnabled ) {
		smsc_log_info(logger, "HttpsOptions initialization. %s", HttpsThreads::diagnostics);
	}
	else {
		smsc_log_info(logger, "HttpsOptions disabled.");
		return 1;
	}
	try {
		/* Load encryption & hashing algorithms for the SSL program */
		SSL_library_init();

		/* Load the error strings for SSL & CRYPTO APIs */
		SSL_load_error_strings();

		  /* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
		if ( method == NULL )
			method = const_cast<SSL_METHOD*>(SSLv23_method());

		result = userInit(user_verify);
		if ( result )
			result = siteInit(site_verify);
#if defined(OPENSSL_THREADS)
		if ( result && cfg->httpsEnabled ) {
			HttpsThreadsSupport = new HttpsThreads();
#endif
		}
	}
	catch(...) {
		result = 0;
		smsc_log_debug(logger, "HttpsOptions::init error");
	}
	return result;
}

int HttpsOptions::userInit(bool verify) {
	std::string tmp;
	userVerify = verify;

	// Create a SSL_CTX structure
	userCtx = SSL_CTX_new(method);
	if (!userCtx) {
		return 0;
	}
	// Set the context mode
	SSL_CTX_set_session_cache_mode(userCtx, SSL_SESS_CACHE_NO_AUTO_CLEAR|SSL_SESS_CACHE_SERVER);
//	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE);
	// Set the workaround options
    SSL_CTX_set_options(userCtx, SSL_OP_ALL);

	// Load the server certificate into the SSL_CTX structure
	tmp = cfg->httpsCertificates + RSA_SERVER_CERT;
	if (SSL_CTX_use_certificate_file(userCtx, tmp.c_str(), SSL_FILETYPE_PEM) <= 0) {
		smsc_log_debug(logger, "HttpsOptions::userInit server certificate usage failed");
		return 0;
	}
	// Load the private-key corresponding to the server certificate
	tmp = cfg->httpsCertificates + RSA_SERVER_KEY;
	if (SSL_CTX_use_PrivateKey_file(userCtx, tmp.c_str(), SSL_FILETYPE_PEM) <= 0) {
		smsc_log_debug(logger, "HttpsOptions::userInit server key usage failed");
		return 0;
    }
	// Check if the server certificate and private-key matches
	if (!SSL_CTX_check_private_key(userCtx)) {
		smsc_log_debug(logger, "HttpsOptions::userInit Server private key does not match the certificate public key");
		return 0;
    }

	// Set the verification depth to 1
	SSL_CTX_set_verify_depth(userCtx,1);
	if ( userVerify ) {
		// Set to require peer (client) certificate verification
		SSL_CTX_set_verify(userCtx, SSL_VERIFY_PEER, NULL);
		// Load the RSA CA certificate into the SSL_CTX structure
		tmp = cfg->httpsCertificates + RSA_SERVER_CA_PATH + RSA_SERVER_CA_CERT;
		if (!SSL_CTX_load_verify_locations(userCtx, tmp.c_str(), NULL)) {
			smsc_log_debug(logger, "HttpsOptions::userInit server certificate verify_locations failed");
			return 0;
		}
	}
	else {
		SSL_CTX_set_verify(userCtx, SSL_VERIFY_NONE, NULL);
	}
    return 1;
}

int HttpsOptions::siteInit(bool verify) {
	std::string tmp;
	siteVerify = verify;

    // Create an SSL_CTX structure
	siteCtx = SSL_CTX_new(method);
	if (!siteCtx) {
		return 0;
	}
	// Set the context mode
	SSL_CTX_set_session_cache_mode(siteCtx, SSL_SESS_CACHE_NO_AUTO_CLEAR|SSL_SESS_CACHE_SERVER);
//	SSL_CTX_set_mode(siteCtx, SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE);
	// Set the workaround options
    SSL_CTX_set_options(siteCtx, SSL_OP_ALL);
	SSL_CTX_set_verify(siteCtx, SSL_VERIFY_NONE, NULL);
/*
    // Load the client certificate into the SSL_CTX structure
	tmp = cfg->httpsCertificates + RSA_CLIENT_CERT;
	if (SSL_CTX_use_certificate_file(siteCtx, tmp.c_str(), SSL_FILETYPE_PEM) <= 0) {
		smsc_log_debug(logger, "HttpsOptions::siteInit client certificate usage failed");
		return 0;
	}
	// Load the private-key corresponding to the server certificate
	tmp = cfg->httpsCertificates + RSA_CLIENT_KEY;
	if (SSL_CTX_use_PrivateKey_file(siteCtx, tmp.c_str(), SSL_FILETYPE_PEM) <= 0) {
		smsc_log_debug(logger, "HttpsOptions::siteInit client private key usage failed");
		return 0;
	}
	// Check if the server certificate and private-key matches
	if (!SSL_CTX_check_private_key(siteCtx)) {
		smsc_log_debug(logger, "HttpsOptions::siteInit Client private key does not match the certificate public key");
		return 0;
	}
	// Set the verification depth to 1
	SSL_CTX_set_verify_depth(siteCtx,1);
	if ( siteVerify ) {
		// Set to require peer (client) certificate verification
		SSL_CTX_set_verify(siteCtx, SSL_VERIFY_PEER, NULL);
		// Load the RSA CA certificate into the SSL_CTX structure
		tmp = cfg->httpsCertificates + RSA_CLIENT_CA_PATH + RSA_CLIENT_CA_CERT;
		if (!SSL_CTX_load_verify_locations(siteCtx, tmp.c_str(), NULL)) {
			smsc_log_debug(logger, "HttpsOptions::siteInit client certificate verify_locations failed");
			return 0;
		}
	}
	else {
		// Set to require peer (client) certificate verification
		SSL_CTX_set_verify(siteCtx, SSL_VERIFY_NONE, NULL);
	}
*/
    return 1;
}

}}}
