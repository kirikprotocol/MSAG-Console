#ifndef	_ResourceLimitationParam_H_
#define	_ResourceLimitationParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ResourceLimitationParam */
typedef struct ResourceLimitationParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ResourceLimitationParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ResourceLimitationParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _ResourceLimitationParam_H_ */
