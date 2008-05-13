#ifndef	_MO_ForwardSM_Arg_H_
#define	_MO_ForwardSM_Arg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SM-RP-DA.h"
#include "SM-RP-OA.h"
#include "SignalInfo.h"
#include "IMSI.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* MO-ForwardSM-Arg */
typedef struct MO_ForwardSM_Arg {
	SM_RP_DA_t	 sm_RP_DA;
	SM_RP_OA_t	 sm_RP_OA;
	SignalInfo_t	 sm_RP_UI;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	IMSI_t	*imsi	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MO_ForwardSM_Arg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MO_ForwardSM_Arg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _MO_ForwardSM_Arg_H_ */
