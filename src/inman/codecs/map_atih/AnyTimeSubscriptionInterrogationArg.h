#ident "$Id$"

#ifndef	_AnyTimeSubscriptionInterrogationArg_H_
#define	_AnyTimeSubscriptionInterrogationArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SubscriberIdentity.h>
#include <RequestedSubscriptionInfo.h>
#include <ISDN-AddressString.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* AnyTimeSubscriptionInterrogationArg */
typedef struct AnyTimeSubscriptionInterrogationArg {
	SubscriberIdentity_t	 subscriberIdentity;
	RequestedSubscriptionInfo_t	 requestedSubscriptionInfo;
	ISDN_AddressString_t	 gsmSCF_Address;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	NULL_t	*longFTN_Supported	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AnyTimeSubscriptionInterrogationArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AnyTimeSubscriptionInterrogationArg;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _AnyTimeSubscriptionInterrogationArg_H_ */
