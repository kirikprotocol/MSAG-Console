#include "Https.h"
#include <openssl/err.h>

namespace scag2 { namespace transport { namespace http {

// todo certificates
#define RSA_SERVER_CERT     "/Users/ksv/tst_data/s_cert.pem"
#define RSA_SERVER_KEY      "/Users/ksv/tst_data/s_privkey.pem"

#define RSA_SERVER_CA_CERT	"server_ca.crt"
#define RSA_SERVER_CA_PATH	"/Users/ksv/tst_data"

#define RSA_CLIENT_CERT		"/Users/ksv/tst_data/c_cert.pem"
#define RSA_CLIENT_KEY		"/Users/ksv/tst_data/c_privkey.pem"

#define RSA_CLIENT_CA_CERT  "client_ca.crt"
#define RSA_CLIENT_CA_PATH  "/Users/ksv/tst_data"

HttpsOptions::~HttpsOptions()
{
	if ( siteCtx )
		SSL_CTX_free(siteCtx);
	if ( userCtx )
		SSL_CTX_free(userCtx);
}

int HttpsOptions::init(bool user_verify, bool site_verify) {
	int result = userInit(user_verify);
	if (result )
		result = siteInit(site_verify);
	return result;
}

int HttpsOptions::userInit(bool verify) {
	userVerify = verify;

	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

    /* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

      /* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	if ( method == NULL )
		method = const_cast<SSL_METHOD*>(SSLv23_method());

	/* Create a SSL_CTX structure */
	userCtx = SSL_CTX_new(method);
	if (!userCtx) {
		return 0;
	}
      /* Load the server certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(userCtx, RSA_SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
		return 0;
	}
	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(userCtx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
		return 0;
    }
	/* Check if the server certificate and private-key matches */
	if (!SSL_CTX_check_private_key(userCtx)) {
//		fprintf(stderr,"Server(user) private key does not match the certificate public key\n");
		return 0;
    }

	if ( userVerify ) {
		/* Load the RSA CA certificate into the SSL_CTX structure */
		if (!SSL_CTX_load_verify_locations(userCtx, RSA_SERVER_CA_CERT, NULL)) {
			return 0;
		}
		/* Set to require peer (client) certificate verification */
		SSL_CTX_set_verify(userCtx,SSL_VERIFY_PEER,NULL);
		/* Set the verification depth to 1 */
		SSL_CTX_set_verify_depth(userCtx,1);
	}

    return 1;
}

int HttpsOptions::siteInit(bool verify) {
	siteVerify = verify;
    /* Create an SSL_CTX structure */
	siteCtx = SSL_CTX_new(method);
	if (!siteCtx) {
		return 0;
	}
    /* Load the client certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(siteCtx, RSA_CLIENT_CERT, SSL_FILETYPE_PEM) <= 0) {
		return 0;
	}
	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(siteCtx, RSA_CLIENT_KEY, SSL_FILETYPE_PEM) <= 0) {
		return 0;
	}
	/* Check if the server certificate and private-key matches */
	if (!SSL_CTX_check_private_key(siteCtx)) {
//		fprintf(stderr,"Client(site) private key does not match the certificate public key\n");
		return 0;
	}
	if ( siteVerify ) {
		/* Load the RSA CA certificate into the SSL_CTX structure */
		if (!SSL_CTX_load_verify_locations(siteCtx, RSA_CLIENT_CA_CERT, NULL)) {
			return 0;
		}
		/* Set to require peer (client) certificate verification */
		SSL_CTX_set_verify(siteCtx, SSL_VERIFY_PEER, NULL);
		/* Set the verification depth to 1 */
		SSL_CTX_set_verify_depth(siteCtx,1);
	}

    return 1;
}

}}}
