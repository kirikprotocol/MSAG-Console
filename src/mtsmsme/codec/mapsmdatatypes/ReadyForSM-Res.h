#ifndef	_ReadyForSM_Res_H_
#define	_ReadyForSM_Res_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ReadyForSM-Res */
typedef struct ReadyForSM_Res {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ReadyForSM_Res_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReadyForSM_Res;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _ReadyForSM_Res_H_ */
