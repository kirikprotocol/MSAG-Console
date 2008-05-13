#ifndef	_RoamingNotAllowedParam_H_
#define	_RoamingNotAllowedParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RoamingNotAllowedCause.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* RoamingNotAllowedParam */
typedef struct RoamingNotAllowedParam {
	RoamingNotAllowedCause_t	 roamingNotAllowedCause;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RoamingNotAllowedParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoamingNotAllowedParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _RoamingNotAllowedParam_H_ */
