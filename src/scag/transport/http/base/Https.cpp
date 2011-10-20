#include "Https.h"
#include <openssl/err.h>

namespace scag2 { namespace transport { namespace http {

// relative to cfg.httpsCertificates
#define RSA_SERVER_CERT     "/s_cert.pem"
#define RSA_SERVER_KEY      "/s_privkey.pem"

#define RSA_SERVER_CA_CERT	"/server_ca.crt"
#define RSA_SERVER_CA_PATH	"/CA"

#define RSA_CLIENT_CERT		"/c_cert.pem"
#define RSA_CLIENT_KEY		"/c_privkey.pem"

#define RSA_CLIENT_CA_CERT  "/client_ca.crt"
#define RSA_CLIENT_CA_PATH  "/CA"

HttpsOptions::~HttpsOptions()
{
	if ( siteCtx )
		SSL_CTX_free(siteCtx);
	if ( userCtx )
		SSL_CTX_free(userCtx);
}

int HttpsOptions::init(bool user_verify, bool site_verify, const config::HttpManagerConfig* conf) {
	this->cfg = conf;
	int result=0;
	logger = Logger::getInstance("http.https");

	userActive = false;
	siteActive = false;
	if ( cfg->httpsEnabled ) {
		smsc_log_info(logger, "HttpsOptions initialization.");
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
		if (result )
			result = siteInit(site_verify);
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
//	SSL_CTX_set_mode(siteCtx, SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE);
	// Set the workaround options
    SSL_CTX_set_options(siteCtx, SSL_OP_ALL);

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

    return 1;
}

}}}
