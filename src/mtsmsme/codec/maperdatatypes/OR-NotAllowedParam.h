#ifndef	_OR_NotAllowedParam_H_
#define	_OR_NotAllowedParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* OR-NotAllowedParam */
typedef struct OR_NotAllowedParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} OR_NotAllowedParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OR_NotAllowedParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _OR_NotAllowedParam_H_ */
