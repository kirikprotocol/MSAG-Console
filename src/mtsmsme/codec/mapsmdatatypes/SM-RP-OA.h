#ifndef	_SM_RP_OA_H_
#define	_SM_RP_OA_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "AddressString.h"
#include <NULL.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SM_RP_OA_PR {
	SM_RP_OA_PR_NOTHING,	/* No components present */
	SM_RP_OA_PR_msisdn,
	SM_RP_OA_PR_serviceCentreAddressOA,
	SM_RP_OA_PR_noSM_RP_OA
} SM_RP_OA_PR;

/* SM-RP-OA */
typedef struct SM_RP_OA {
	SM_RP_OA_PR present;
	union SM_RP_OA_u {
		ISDN_AddressString_t	 msisdn;
		AddressString_t	 serviceCentreAddressOA;
		NULL_t	 noSM_RP_OA;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SM_RP_OA_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_RP_OA;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_RP_OA_H_ */
