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
#include <openssl/conf.h>
#include <openssl/engine.h>
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
//	smsc_log_debug(smsc::logger::Logger::getInstance("http.threads"), "%p setup. CRYPTO_num_locks=%d", this, CRYPTO_num_locks());
	mutex_buf = (pthread_mutex_t*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	lock_count = (long*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	if (!mutex_buf)
		return 0;
	for (int i = 0; i < CRYPTO_num_locks(); i++) {
		lock_count[i]=0;
		pthread_mutex_init(&mutex_buf[i], NULL);
	}
	// callbacks for static lock
	CRYPTO_set_id_callback(/*(unsigned long (*)())*/thread_id_function);
	CRYPTO_set_locking_callback(/*(void (*)())*/locking_function);

/*
    // TODO: callbacks for dynamic lock (openSSL version 1.0.0 and later)
    CRYPTO_set_dynlock_create_callback(dyn_lock_create_function);
    CRYPTO_set_dynlock_destroy_callback(dyn_lock_destroy_function);
    CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
*/
	return 1;
}
int HttpsThreads::cleanup() {
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
/*
	// TODO: callbacks for dynamic lock (openSSL version 1.0.0 and later)
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
*/

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
//	smsc_log_debug(smsc::logger::Logger::getInstance("http.threads"), "lock thread:%d %s:%s mode:%d type:%d %s:%d", CRYPTO_thread_id(),
//			(mode&CRYPTO_LOCK)?"l":"u",	(type&CRYPTO_READ)?"r":"w", mode, type, file, line);

	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&mutex_buf[type]);
		lock_count[type]++;
	}
	else {
		pthread_mutex_unlock(&mutex_buf[type]);
	}
}

/*
// TODO: -----------------------------------------------------------------------------------------------
CRYPTO_dynlock_value* HttpsThreads::dyn_lock_create_function(const char* file, int line) {
    return new CRYPTO_dynlock_value;
}

void HttpsThreads::dyn_lock_destroy_function(CRYPTO_dynlock_value* l, const char* file, int line) {
    delete l;
}

void HttpsThreads::dyn_lock_function(int mode, CRYPTO_dynlock_value* l, const char* file, int line) {
    if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&l->mtx);
    else
		pthread_mutex_unlock(&(l->mtx) );
}
*/
//-----------------------------------------------------------------------------------------------------
HttpsOptions::~HttpsOptions()
{
	if ( siteCtx ) {
		SSL_CTX_free(siteCtx);
		if (logger->isDebugEnabled()) CRYPTO_mem_leaks_fp(stdout); //openssl memory leaks internal detection
		siteCtx = NULL;
	}
	if ( userCtx ) {
		SSL_CTX_free(userCtx);
		if (logger->isDebugEnabled()) CRYPTO_mem_leaks_fp(stdout); //openssl memory leaks internal detection
		userCtx = NULL;
	}
	if ( HttpsThreadsSupport ) {
		delete HttpsThreadsSupport;
		HttpsThreadsSupport = NULL;
	}
	if (logger->isDebugEnabled()) {
		CRYPTO_mem_leaks_fp(stdout); //openssl memory leaks internal detection
		MemCheck_stop();
	}
}

void HttpsOptions::shutdown() {
	statistics();
	ERR_remove_state(0);
	ENGINE_cleanup();
	CONF_modules_unload(1);

	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
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
		// Load encryption & hashing algorithms for the SSL program
		SSL_library_init();

		//openssl memory leaks internal detection
		if (logger->isDebugEnabled()) MemCheck_start();

		// Load the error strings for SSL & CRYPTO APIs
		SSL_load_error_strings();

		// Create a SSL_METHOD structure (choose a SSL/TLS protocol version)
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
		//SSL_CTX_flush_sessions()
		// SSL_CTX_set_session_cache_mode(),
		// ssl->ctx->session_cache_mode&SSL_SESS_CACHE_NO_INTERNAL_LOOK UP
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
	SSL_CTX_set_session_cache_mode(userCtx, SSL_SESS_CACHE_NO_AUTO_CLEAR|SSL_SESS_CACHE_OFF); // SSL_SESS_CACHE_SERVER);
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
	SSL_CTX_set_session_cache_mode(siteCtx, SSL_SESS_CACHE_NO_AUTO_CLEAR|SSL_SESS_CACHE_OFF); // SSL_SESS_CACHE_CLIENT);
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

void HttpsOptions::statistics(void) {
	if ( userActive )
		statSessions(userCtx, sstUser, "user");
	if ( siteActive )
		statSessions(siteCtx, sstSite, "site");
}

void HttpsOptions::statSessions(SSL_CTX* ctx, sslStatistics& st, const char* id) {
	time_t cur_time = time(0);
	if ( (cur_time - st.ss_time) < 10 ) return;
	st.ss_time = cur_time;
/*
	st.ss_number = SSL_CTX_sess_number(ctx);
	st.ss_connect = SSL_CTX_sess_connect(ctx);
	st.ss_connect_good = SSL_CTX_sess_connect_good(ctx);
	st.ss_connect_renegotiate = SSL_CTX_sess_connect_renegotiate(ctx);
	st.ss_accept = SSL_CTX_sess_accept(ctx);
	st.ss_accept_good = SSL_CTX_sess_accept_good(ctx);
	st.ss_accept_renegotiate = SSL_CTX_sess_accept_renegotiate(ctx);
	st.ss_hits = SSL_CTX_sess_hits(ctx);
	st.ss_cb_hits = SSL_CTX_sess_cb_hits(ctx);
	st.ss_misses = SSL_CTX_sess_misses(ctx);
	st.ss_timeouts = SSL_CTX_sess_timeouts(ctx);
	st.ss_cache_full = SSL_CTX_sess_cache_full(ctx);
*/
	smsc_log_debug(logger, "%s statistics %d conn %d/%d/%d accept %d/%d/%d hits %d/%d miss %d to %d full %d", id,
		SSL_CTX_sess_number(ctx),
		SSL_CTX_sess_connect(ctx),
		SSL_CTX_sess_connect_good(ctx),
		SSL_CTX_sess_connect_renegotiate(ctx),
		SSL_CTX_sess_accept(ctx),
		SSL_CTX_sess_accept_good(ctx),
		SSL_CTX_sess_accept_renegotiate(ctx),
		SSL_CTX_sess_hits(ctx),
		SSL_CTX_sess_cb_hits(ctx),
		SSL_CTX_sess_misses(ctx),
		SSL_CTX_sess_timeouts(ctx),
		SSL_CTX_sess_cache_full(ctx)
/*
		st.ss_number,
		st.ss_connect,
		st.ss_connect_good,
		st.ss_connect_renegotiate,
		st.ss_accept,
		st.ss_accept_good,
		st.ss_accept_renegotiate,
		st.ss_hits,
		st.ss_cb_hits,
		st.ss_misses,
		st.ss_timeouts,
		st.ss_cache_full
*/
	);
	SSL_CTX_flush_sessions(ctx, cur_time);
}
/*
// function to generate an X509-format certificate and RSA public key. ItÕs signed by a private key.
static X509* HttpsOptions::create_certificate(RSA* rsa, RSA* rsaSign, const char* cname, const char* cnameSign, const char* pszOrgName, unsigned int certLifetime) {
    time_t start_time = time(NULL);
    time_t end_time = start_time + certLifetime;

    EVP_PKEY* sign_pkey = get_evp_pkey(rsaSign, 1);
    if (!sign_pkey)
        goto error;

    EVP_PKEY* pkey = get_evp_pkey(rsa, 0);
    if (!pkey)
        goto error;

    X509* x509 = X509_new();
    if (!x509)
        goto error;
    if (!(X509_set_version(x509, 2)))
        goto error;
    if (!(ASN1_INTEGER_set(X509_get_serialNumber(x509), (long)start_time)))
        goto error;

    X509_NAME* name = X509_NAME_new();
    if (!name)
        goto error;

    int nid = OBJ_txt2nid("organizationName");
    if (nid == NID_undef)
        goto error;
    if (!(X509_NAME_add_entry_by_NID(name, nid, MBSTRING_ASC, (unsigned char*)pszOrgName, -1, -1, 0)))
        goto error;
    if ((nid = OBJ_txt2nid("commonName")) == NID_undef)
        goto error;
    if (!(X509_NAME_add_entry_by_NID(name, nid, MBSTRING_ASC, (unsigned char*)cname, -1, -1, 0)))
        goto error;
    if (!(X509_set_subject_name(x509, name)))
        goto error;

    X509_NAME* name_issuer = X509_NAME_new();
    if (!name_issuer)
        goto error;
    if ((nid = OBJ_txt2nid("organizationName")) == NID_undef)
        goto error;
    if (!(X509_NAME_add_entry_by_NID(name_issuer, nid, MBSTRING_ASC, (unsigned char*)pszOrgName, -1, -1, 0)))
        goto error;
    if ((nid = OBJ_txt2nid("commonName")) == NID_undef)
        goto error;
    if (!(X509_NAME_add_entry_by_NID(name_issuer, nid, MBSTRING_ASC, (unsigned char*)cnameSign, -1, -1, 0)))
        goto error;
    if (!(X509_set_issuer_name(x509, name_issuer)))
        goto error;

    if (!X509_time_adj(X509_get_notBefore(x509), 0, &start_time))
        goto error;

    if (!X509_time_adj(X509_get_notAfter(x509), 0, &end_time))
        goto error;
    if (!X509_set_pubkey(x509, pkey))
        goto error;
    if (!X509_sign(x509, sign_pkey, EVP_sha1()))
        goto error;

    goto done;

error:
    if (x509) {
        X509_free(x509);
        x509 = NULL;
    }

done:
    if (sign_pkey)
        EVP_PKEY_free(sign_pkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (name)
        X509_NAME_free(name);
    if (name_issuer)
        X509_NAME_free(name_issuer);

    return x509;
}


// Key-pair/certificate generation for public key en/decryption. This sample application uses a self-signed certificate to omit the need of a valid certificate.
//const char* certName = "test_name";
int HttpsOptions::genKeyCert(const char* certName) {
	int result = 0;
	do {
		RSA* rsa = RSA_generate_key(1024, 65537, NULL, NULL);
		if (!rsa) {
			smsc_log_error(logger, "RSA_generate_key error");
			break;
		}

		const char* certName = "test name";
		X509* cert = create_certificate(rsa, rsa, certName, certName, "DICE", 3 * 365 * 24 * 60 * 60);  //3 years
		if (!cert) {
			smsc_log_error(logger, "Couldn't create a certificate");
			break;
		}

		if (SSL_CTX_use_RSAPrivateKey(ctx, rsa) <= 0) {
			smsc_log_error(logger, "Couldn't use RSAPrivateKey"); //ERR_print_errors_fp(stderr);
			break;
		}

		if (SSL_CTX_use_certificate(ctx, cert) <= 0) {
			smsc_log_error(logger, "Couldn't use certificate");	//ERR_print_errors_fp(stderr);
			break;
		}

		RSA_free(rsa);

		X509_free(cert);

		if (!SSL_CTX_check_private_key(ctx)) {
			smsc_log_error(logger, "Private key is invalid");
			break;
		}
		smsc_log_error(logger, "Private key is correct");
		result = 1;
	} while (0);
	return result;
}
*/

}}}
