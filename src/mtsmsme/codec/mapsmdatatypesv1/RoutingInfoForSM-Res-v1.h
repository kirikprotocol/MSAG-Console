#ifndef	_RoutingInfoForSM_Res_v1_H_
#define	_RoutingInfoForSM_Res_v1_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include <BOOLEAN.h>
#include "ForwardingData-v1.h"
#include "LocationInfo-v1.h"
#include "LMsId-v1.h"
#include <constr_SEQUENCE.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum choice_PR {
	choice_PR_NOTHING,	/* No components present */
	choice_PR_sequence,
	choice_PR_forwardingData
} choice_PR;

/* RoutingInfoForSM-Res-v1 */
typedef struct RoutingInfoForSM_Res_v1 {
	IMSI_t	 imsi;
	struct choice {
		choice_PR present;
		union RoutingInfoForSM_Res_v1__choice_u {
			struct sequence {
				LocationInfo_v1_t	 locationInfo;
				LMsId_v1_t	*lMsId	/* OPTIONAL */;
				
				/* Context for parsing across buffer boundaries */
				asn_struct_ctx_t _asn_ctx;
			} sequence;
			ForwardingData_v1_t	 forwardingData;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} choice;
	BOOLEAN_t	*mwd_Set	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RoutingInfoForSM_Res_v1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Res_v1;

#ifdef __cplusplus
}
#endif

#endif	/* _RoutingInfoForSM_Res_v1_H_ */
