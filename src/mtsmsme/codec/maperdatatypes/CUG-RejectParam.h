#ifndef	_CUG_RejectParam_H_
#define	_CUG_RejectParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include "CUG-RejectCause.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CUG-RejectParam */
typedef struct CUG_RejectParam {
	CUG_RejectCause_t	*cug_RejectCause	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CUG_RejectParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CUG_RejectParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _CUG_RejectParam_H_ */
