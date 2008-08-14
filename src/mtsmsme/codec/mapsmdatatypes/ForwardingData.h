#ifndef	_ForwardingData_H_
#define	_ForwardingData_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "ForwardingOptions.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ForwardingData */
typedef struct ForwardingData {
	ISDN_AddressString_t	 forwardedToNumber;
	ForwardingOptions_t	*forwardingOptions	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ForwardingData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingData;

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingData_H_ */
