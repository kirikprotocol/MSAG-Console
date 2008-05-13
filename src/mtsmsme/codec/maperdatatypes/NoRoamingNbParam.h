#ifndef	_NoRoamingNbParam_H_
#define	_NoRoamingNbParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* NoRoamingNbParam */
typedef struct NoRoamingNbParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NoRoamingNbParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NoRoamingNbParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _NoRoamingNbParam_H_ */
