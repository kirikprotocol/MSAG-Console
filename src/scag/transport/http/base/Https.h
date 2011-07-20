#ifndef SCAG_TRANSPORT_HTTP_BASE_HTTPS
#define SCAG_TRANSPORT_HTTP_BASE_HTTPS

#include <openssl/ssl.h>
#include "logger/Logger.h"
#include <string>

namespace scag2 { namespace transport { namespace http {
using smsc::logger::Logger;

#define VALIDATE_CERT true
#define NO_VALIDATE_CERT false

class HttpsOptions
{
public:
	HttpsOptions() : method(NULL), userCtx(NULL), siteCtx(NULL) {};
	HttpsOptions(SSL_METHOD *meth) : method(meth), userCtx(NULL), siteCtx(NULL) {};
    ~HttpsOptions();

    int init(bool user_verify, bool site_verify, std::string certDir);
    SSL_CTX* userContext(void) { return userCtx; }
    SSL_CTX* siteContext(void) { return siteCtx; }

    bool		userVerify;
    bool		siteVerify;

protected:
    Logger *logger;
    std::string certificatesDir;
    SSL_METHOD* method;
    SSL_CTX*	userCtx;
    SSL_CTX*	siteCtx;
    int userInit(bool verify);
    int siteInit(bool verify);
};

}}}

#endif // SCAG_TRANSPORT_HTTP_BASE_HTTPS
