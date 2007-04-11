#ifndef	_AARE_apdu_H_
#define	_AARE_apdu_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <OBJECT_IDENTIFIER.h>
#include "Associate-result.h"
#include "Associate-source-diagnostic.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MEXT;

/* AARE-apdu */
typedef struct AARE_apdu {
	BIT_STRING_t	*protocol_version	/* OPTIONAL */;
	OBJECT_IDENTIFIER_t	 application_context_name;
	Associate_result_t	 result;
	Associate_source_diagnostic_t	 result_source_diagnostic;
	struct aare_user_information {
		A_SEQUENCE_OF(struct MEXT) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *aare_user_information;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AARE_apdu_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AARE_apdu;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "MEXT.h"

#endif	/* _AARE_apdu_H_ */
