#ifndef	_SM_RP_DA_H_
#define	_SM_RP_DA_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "LMSI.h"
#include "AddressString.h"
#include <NULL.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SM_RP_DA_PR {
	SM_RP_DA_PR_NOTHING,	/* No components present */
	SM_RP_DA_PR_imsi,
	SM_RP_DA_PR_lmsi,
	SM_RP_DA_PR_serviceCentreAddressDA,
	SM_RP_DA_PR_noSM_RP_DA
} SM_RP_DA_PR;

/* SM-RP-DA */
typedef struct SM_RP_DA {
	SM_RP_DA_PR present;
	union SM_RP_DA_u {
		IMSI_t	 imsi;
		LMSI_t	 lmsi;
		AddressString_t	 serviceCentreAddressDA;
		NULL_t	 noSM_RP_DA;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SM_RP_DA_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_RP_DA;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_RP_DA_H_ */
