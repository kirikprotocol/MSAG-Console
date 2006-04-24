#ident "$Id$"

#ifndef	_RequestedSubscriptionInfo_H_
#define	_RequestedSubscriptionInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <RequestedCAMEL-SubscriptionInfo.h>
#include <AdditionalRequestedCAMEL-SubscriptionInfo.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SS_ForBS_Code;
struct ExtensionContainer;

/* RequestedSubscriptionInfo */
typedef struct RequestedSubscriptionInfo {
	struct SS_ForBS_Code	*requestedSS_Info	/* OPTIONAL */;
	NULL_t	*odb	/* OPTIONAL */;
	RequestedCAMEL_SubscriptionInfo_t	*requestedCAMEL_SubscriptionInfo	/* OPTIONAL */;
	NULL_t	*supportedVLR_CAMEL_Phases	/* OPTIONAL */;
	NULL_t	*supportedSGSN_CAMEL_Phases	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AdditionalRequestedCAMEL_SubscriptionInfo_t	*additionalRequestedCAMEL_SubscriptionInfo	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RequestedSubscriptionInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RequestedSubscriptionInfo;

/* Referred external types */
#include <SS-ForBS-Code.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _RequestedSubscriptionInfo_H_ */
