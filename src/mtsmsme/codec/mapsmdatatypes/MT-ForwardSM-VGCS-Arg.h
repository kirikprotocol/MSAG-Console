#ifndef	_MT_ForwardSM_VGCS_Arg_H_
#define	_MT_ForwardSM_VGCS_Arg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ASCI-CallReference.h"
#include "SM-RP-OA.h"
#include "SignalInfo.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* MT-ForwardSM-VGCS-Arg */
typedef struct MT_ForwardSM_VGCS_Arg {
	ASCI_CallReference_t	 asciCallReference;
	SM_RP_OA_t	 sm_RP_OA;
	SignalInfo_t	 sm_RP_UI;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MT_ForwardSM_VGCS_Arg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_ForwardSM_VGCS_Arg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _MT_ForwardSM_VGCS_Arg_H_ */
