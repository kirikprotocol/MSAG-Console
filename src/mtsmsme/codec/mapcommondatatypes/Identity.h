#ifndef	_Identity_H_
#define	_Identity_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "IMSI-WithLMSI.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Identity_PR {
	Identity_PR_NOTHING,	/* No components present */
	Identity_PR_imsi,
	Identity_PR_imsi_WithLMSI
} Identity_PR;

/* Identity */
typedef struct Identity {
	Identity_PR present;
	union Identity_u {
		IMSI_t	 imsi;
		IMSI_WithLMSI_t	 imsi_WithLMSI;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Identity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Identity;

#ifdef __cplusplus
}
#endif

#endif	/* _Identity_H_ */
