#ifndef	_RoutingInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_RoutingInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ISDN-AddressString.h>
#include <ForwardingData.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RoutingInfo_PR {
	RoutingInfo_PR_NOTHING,	/* No components present */
	RoutingInfo_PR_roamingNumber,
	RoutingInfo_PR_forwardingData
} RoutingInfo_PR;

/* RoutingInfo */
typedef struct RoutingInfo {
	RoutingInfo_PR present;
	union RoutingInfo_u {
		ISDN_AddressString_t	 roamingNumber;
		ForwardingData_t	 forwardingData;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RoutingInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoutingInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _RoutingInfo_H_ */
