#ifndef	_ABRT_apdu_H_
#define	_ABRT_apdu_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ABRT-source.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MEXT;

/* ABRT-apdu */
typedef struct ABRT_apdu {
	ABRT_source_t	 abort_source;
	struct abrt_user_information {
		A_SEQUENCE_OF(struct MEXT) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *abrt_user_information;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ABRT_apdu_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ABRT_apdu;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "MEXT.h"

#endif	/* _ABRT_apdu_H_ */
