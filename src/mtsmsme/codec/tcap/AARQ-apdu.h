#ifndef	_AARQ_apdu_H_
#define	_AARQ_apdu_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <OBJECT_IDENTIFIER.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MEXT;

/* AARQ-apdu */
typedef struct AARQ_apdu {
	BIT_STRING_t	*protocol_version	/* OPTIONAL */;
	OBJECT_IDENTIFIER_t	 application_context_name;
	struct aarq_user_information {
		A_SEQUENCE_OF(struct MEXT) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *aarq_user_information;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AARQ_apdu_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AARQ_apdu;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "MEXT.h"

#endif	/* _AARQ_apdu_H_ */
