#ifndef	_RoutingInfoForSM_Res_H_
#define	_RoutingInfoForSM_Res_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "LocationInfoWithLMSI.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* RoutingInfoForSM-Res */
typedef struct RoutingInfoForSM_Res {
	IMSI_t	 imsi;
	LocationInfoWithLMSI_t	 locationInfoWithLMSI;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RoutingInfoForSM_Res_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Res;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _RoutingInfoForSM_Res_H_ */
