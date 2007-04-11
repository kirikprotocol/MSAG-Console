#ifndef	_SuperChargerInfo_H_
#define	_SuperChargerInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include "AgeIndicator.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SuperChargerInfo_PR {
	SuperChargerInfo_PR_NOTHING,	/* No components present */
	SuperChargerInfo_PR_sendSubscriberData,
	SuperChargerInfo_PR_subscriberDataStored
} SuperChargerInfo_PR;

/* SuperChargerInfo */
typedef struct SuperChargerInfo {
	SuperChargerInfo_PR present;
	union SuperChargerInfo_u {
		NULL_t	 sendSubscriberData;
		AgeIndicator_t	 subscriberDataStored;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SuperChargerInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SuperChargerInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _SuperChargerInfo_H_ */
