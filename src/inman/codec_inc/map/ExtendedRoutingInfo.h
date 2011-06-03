#ifndef	_ExtendedRoutingInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ExtendedRoutingInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <RoutingInfo.h>
#include <CamelRoutingInfo.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ExtendedRoutingInfo_PR {
	ExtendedRoutingInfo_PR_NOTHING,	/* No components present */
	ExtendedRoutingInfo_PR_routingInfo,
	ExtendedRoutingInfo_PR_camelRoutingInfo
} ExtendedRoutingInfo_PR;

/* ExtendedRoutingInfo */
typedef struct ExtendedRoutingInfo {
	ExtendedRoutingInfo_PR present;
	union ExtendedRoutingInfo_u {
		RoutingInfo_t	 routingInfo;
		CamelRoutingInfo_t	 camelRoutingInfo;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ExtendedRoutingInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ExtendedRoutingInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _ExtendedRoutingInfo_H_ */
