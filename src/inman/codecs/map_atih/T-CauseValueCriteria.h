#ident "$Id$"

#ifndef	_T_CauseValueCriteria_H_
#define	_T_CauseValueCriteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CauseValue.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* T-CauseValueCriteria */
typedef struct T_CauseValueCriteria {
	A_SEQUENCE_OF(CauseValue_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} T_CauseValueCriteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_CauseValueCriteria;

#ifdef __cplusplus
}
#endif

#endif	/* _T_CauseValueCriteria_H_ */
