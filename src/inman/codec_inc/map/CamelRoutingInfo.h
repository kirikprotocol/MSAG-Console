#ifndef	_CamelRoutingInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CamelRoutingInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <GmscCamelSubscriptionInfo.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ForwardingData;
struct ExtensionContainer;

/* CamelRoutingInfo */
typedef struct CamelRoutingInfo {
	struct ForwardingData	*forwardingData	/* OPTIONAL */;
	GmscCamelSubscriptionInfo_t	 gmscCamelSubscriptionInfo;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CamelRoutingInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CamelRoutingInfo;

/* Referred external types */
#include <ForwardingData.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CamelRoutingInfo_H_ */
