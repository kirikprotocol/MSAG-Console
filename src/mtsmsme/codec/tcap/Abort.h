#ifndef	_Abort_H_
#define	_Abort_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DestTransactionID.h"
#include "P-AbortCause.h"
#include "DialoguePortion.h"
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum reason_PR {
	reason_PR_NOTHING,	/* No components present */
	reason_PR_p_abortCause,
	reason_PR_u_abortCause
} reason_PR;

/* Abort */
typedef struct Abort {
	DestTransactionID_t	 dtid;
	struct reason {
		reason_PR present;
		union Abort__reason_u {
			P_AbortCause_t	 p_abortCause;
			DialoguePortion_t	 u_abortCause;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *reason;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Abort_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Abort;

#ifdef __cplusplus
}
#endif

#endif	/* _Abort_H_ */
