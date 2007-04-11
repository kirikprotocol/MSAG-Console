#ifndef	_IllegalSS_OperationParam_H_
#define	_IllegalSS_OperationParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* IllegalSS-OperationParam */
typedef struct IllegalSS_OperationParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IllegalSS_OperationParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IllegalSS_OperationParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _IllegalSS_OperationParam_H_ */
