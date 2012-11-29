#ifndef SCAG_TRANSPORT_HTTP_BASE_HTTPS
#define SCAG_TRANSPORT_HTTP_BASE_HTTPS

#include <string>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include "logger/Logger.h"
#include "scag/config/http/HttpManagerConfig.h"

namespace scag2 { namespace transport { namespace http {
using smsc::logger::Logger;

#define VALIDATE_CERT true
#define NO_VALIDATE_CERT false

struct CRYPTO_dynlock_value
{
	pthread_mutex_t mtx;
};

class HttpsThreads
{
private:
	HttpsThreads(HttpsThreads& ) {};
public:
	HttpsThreads() { setup(); }
	~HttpsThreads() { cleanup(); }

	static void locking_function(int mode, int type, const char *file, int line);
	static unsigned long thread_id_function(void) { return (unsigned long)pthread_self(); }

/* // TODO: dynamic lock
	static CRYPTO_dynlock_value* dyn_lock_create_function(const char* file, int line);
	static void dyn_lock_destroy_function(CRYPTO_dynlock_value* l, const char* file, int line);
	static void dyn_lock_function(int mode, CRYPTO_dynlock_value* l, const char* file, int line);
*/
	int setup();
	int cleanup();

public:
	static const char* diagnostics;

protected:
	static pthread_mutex_t* mutex_buf;
	static long* lock_count;
};

class HttpsOptions
{
public:
	HttpsOptions()
		: userVerify(false), siteVerify(false)
		, userActive(false), siteActive(false)
		, cfg(NULL), logger(NULL)
		, method(NULL), userCtx(NULL), siteCtx(NULL)
		{};
	HttpsOptions(SSL_METHOD* meth)
		: userVerify(false), siteVerify(false)
		, userActive(false), siteActive(false)
		, cfg(NULL), logger(NULL)
		, method(meth), userCtx(NULL), siteCtx(NULL)
		{};
	HttpsOptions(HttpsOptions& src)
		: userVerify(src.userVerify)
		, siteVerify(src.siteVerify)
		, userActive(src.userActive)
		, siteActive(src.siteActive)
		, cfg(src.cfg)
		, logger(src.logger)
		, method(src.method)
		, userCtx(src.userCtx)
		, siteCtx(src.siteCtx)
		{};
    ~HttpsOptions();

    int init(bool user_verify, bool site_verify, const config::HttpManagerConfig* conf);
    void shutdown();
    SSL_CTX* userContext(void) { return userCtx; }
    SSL_CTX* siteContext(void) { return siteCtx; }
    int httpsTimeout(void) { return cfg->httpsTimeout; }
    void statistics(void);

    bool		userVerify;
    bool		siteVerify;
    bool		userActive;
    bool		siteActive;

    static HttpsThreads* HttpsThreadsSupport;

protected:
    const config::HttpManagerConfig* cfg;
    Logger*		logger;
    SSL_METHOD* method;
    SSL_CTX*	userCtx;
    SSL_CTX*	siteCtx;
    int userInit(bool verify);
    int siteInit(bool verify);
    struct sslStatistics {
    	time_t ss_time;
/*
    	long ss_number; 			// current number of sessions in the internal session cache.
    	long ss_connect;			// number of started SSL/TLS handshakes in client mode.
    	long ss_connect_good;		// number of successfully established SSL/TLS sessions in client mode.
    	long ss_connect_renegotiate;// number of start renegotiations in client mode.
    	long ss_accept;				// number of started SSL/TLS handshakes in server mode.
    	long ss_accept_good;		// number of successfully established SSL/TLS sessions in server mode.
    	long ss_accept_renegotiate;	// number of start renegotiations in server mode.
    	long ss_hits;				// number of successfully reused sessions. In client mode a session set with SSL_set_session(3) successfully reused is counted as a hit. In server mode a session successfully retrieved from longernal or external cache is counted as a hit.
    	long ss_cb_hits;			// number of successfully retrieved sessions from the external session cache in server mode.
    	long ss_misses;				// number of sessions proposed by clients that were not found in the internal session cache in server mode.
    	long ss_timeouts;			// number of sessions proposed by clients and either found in the internal or external session cache in server mode, but that were invalid due to timeout. These sessions are not included in the long ss_hits() count.
    	long ss_cache_full;			// number of sessions that were removed because the maximum session cache size was exceeded.
*/
    	sslStatistics() : ss_time(time(0)) {}
    } sstUser, sstSite;
    void statSessions(SSL_CTX* ctx, sslStatistics& st, const char* id);

    static const char* openssl_thread_diag;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_BASE_HTTPS
