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

class HttpsThreads
{
private:
	HttpsThreads(HttpsThreads& ) {};
public:
	HttpsThreads() { setup(); }
	~HttpsThreads() { cleanup(); }

	static void locking_function(int mode, int type, const char *file, int line);
	static unsigned long thread_id_function(void) { return (unsigned long)pthread_self(); }

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
private:
	HttpsOptions()
		: userVerify(false), siteVerify(false)
		, userActive(false), siteActive(false)
		, cfg(NULL), logger(NULL)
		, method(NULL), userCtx(NULL), siteCtx(NULL)
		{};
public:
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
    SSL_CTX* userContext(void) { return userCtx; }
    SSL_CTX* siteContext(void) { return siteCtx; }
    int httpsTimeout(void) { return cfg->httpsTimeout; }

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

    static const char* openssl_thread_diag;
};

}}}

#endif // SCAG_TRANSPORT_HTTP_BASE_HTTPS
