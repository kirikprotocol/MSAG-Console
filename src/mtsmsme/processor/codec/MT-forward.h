#ifndef	_MT_forward_H_
#define	_MT_forward_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <NULL.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum sm_RP_DA_PR {
	sm_RP_DA_PR_NOTHING,	/* No components present */
	sm_RP_DA_PR_imsi,
	sm_RP_DA_PR_lmsi,
	sm_RP_DA_PR_serviceCentreAddressDA,
	sm_RP_DA_PR_noSM_RP_DA
} sm_RP_DA_PR;
typedef enum sm_RP_OA_PR {
	sm_RP_OA_PR_NOTHING,	/* No components present */
	sm_RP_OA_PR_msisdn,
	sm_RP_OA_PR_serviceCentreAddressOA,
	sm_RP_OA_PR_noSM_RP_OA
} sm_RP_OA_PR;

/* MT-forward */
typedef struct MT_forward {
	struct sm_RP_DA {
		sm_RP_DA_PR present;
		union MT_forward__sm_RP_DA_u {
			OCTET_STRING_t	 imsi;
			OCTET_STRING_t	 lmsi;
			OCTET_STRING_t	 serviceCentreAddressDA;
			NULL_t	 noSM_RP_DA;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} sm_RP_DA;
	struct sm_RP_OA {
		sm_RP_OA_PR present;
		union MT_forward__sm_RP_OA_u {
			OCTET_STRING_t	 msisdn;
			OCTET_STRING_t	 serviceCentreAddressOA;
			NULL_t	 noSM_RP_OA;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} sm_RP_OA;
	OCTET_STRING_t	 sm_RP_UI;
	NULL_t	*moreMessagesToSend	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MT_forward_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_forward;

#ifdef __cplusplus
}
#endif

#endif	/* _MT_forward_H_ */
