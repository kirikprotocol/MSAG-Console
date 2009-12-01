#ifndef	_LocationInfo_v1_H_
#define	_LocationInfo_v1_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum LocationInfo_v1_PR {
	LocationInfo_v1_PR_NOTHING,	/* No components present */
	LocationInfo_v1_PR_roamingNumber,
	LocationInfo_v1_PR_mscNumber
} LocationInfo_v1_PR;

/* LocationInfo-v1 */
typedef struct LocationInfo_v1 {
	LocationInfo_v1_PR present;
	union LocationInfo_v1_u {
		ISDN_AddressString_t	 roamingNumber;
		ISDN_AddressString_t	 mscNumber;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInfo_v1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInfo_v1;

#ifdef __cplusplus
}
#endif

#endif	/* _LocationInfo_v1_H_ */
