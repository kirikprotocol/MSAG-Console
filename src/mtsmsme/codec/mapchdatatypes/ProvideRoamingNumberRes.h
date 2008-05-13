#ifndef	_ProvideRoamingNumberRes_H_
#define	_ProvideRoamingNumberRes_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ProvideRoamingNumberRes */
typedef struct ProvideRoamingNumberRes {
	ISDN_AddressString_t	 roamingNumber;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*releaseResourcesSupported	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProvideRoamingNumberRes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProvideRoamingNumberRes;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _ProvideRoamingNumberRes_H_ */
