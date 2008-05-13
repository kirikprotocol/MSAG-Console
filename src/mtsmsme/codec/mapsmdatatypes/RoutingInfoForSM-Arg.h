#ifndef	_RoutingInfoForSM_Arg_H_
#define	_RoutingInfoForSM_Arg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <BOOLEAN.h>
#include "AddressString.h"
#include <NULL.h>
#include "SM-RP-MTI.h"
#include "SM-RP-SMEA.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* RoutingInfoForSM-Arg */
typedef struct RoutingInfoForSM_Arg {
	ISDN_AddressString_t	 msisdn;
	BOOLEAN_t	 sm_RP_PRI;
	AddressString_t	 serviceCentreAddress;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*gprsSupportIndicator	/* OPTIONAL */;
	SM_RP_MTI_t	*sm_RP_MTI	/* OPTIONAL */;
	SM_RP_SMEA_t	*sm_RP_SMEA	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RoutingInfoForSM_Arg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Arg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _RoutingInfoForSM_Arg_H_ */
