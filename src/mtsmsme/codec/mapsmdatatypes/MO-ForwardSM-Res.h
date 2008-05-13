#ifndef	_MO_ForwardSM_Res_H_
#define	_MO_ForwardSM_Res_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SignalInfo.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* MO-ForwardSM-Res */
typedef struct MO_ForwardSM_Res {
	SignalInfo_t	*sm_RP_UI	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MO_ForwardSM_Res_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MO_ForwardSM_Res;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _MO_ForwardSM_Res_H_ */
