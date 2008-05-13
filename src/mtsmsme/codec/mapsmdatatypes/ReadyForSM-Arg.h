#ifndef	_ReadyForSM_Arg_H_
#define	_ReadyForSM_Arg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "AlertReason.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ReadyForSM-Arg */
typedef struct ReadyForSM_Arg {
	IMSI_t	 imsi;
	AlertReason_t	 alertReason;
	NULL_t	*alertReasonIndicator	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ReadyForSM_Arg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReadyForSM_Arg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _ReadyForSM_Arg_H_ */
