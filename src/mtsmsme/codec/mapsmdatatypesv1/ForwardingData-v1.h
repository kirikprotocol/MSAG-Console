#ifndef	_ForwardingData_v1_H_
#define	_ForwardingData_v1_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "ForwardingOptions-v1.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ForwardingData-v1 */
typedef struct ForwardingData_v1 {
	ISDN_AddressString_t	 forwardedToNumber;
	ForwardingOptions_v1_t	*forwardingOptions	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ForwardingData_v1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingData_v1;

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingData_v1_H_ */
