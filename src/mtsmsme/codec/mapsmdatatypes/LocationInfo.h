#ifndef	_LocationInfo_H_
#define	_LocationInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum LocationInfo_PR {
	LocationInfo_PR_NOTHING,	/* No components present */
	LocationInfo_PR_roamingNumber,
	LocationInfo_PR_mscNumber
} LocationInfo_PR;

/* LocationInfo */
typedef struct LocationInfo {
	LocationInfo_PR present;
	union LocationInfo_u {
		ISDN_AddressString_t	 roamingNumber;
		ISDN_AddressString_t	 mscNumber;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _LocationInfo_H_ */
