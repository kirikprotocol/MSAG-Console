#ifndef	_O_CauseValueCriteria_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_O_CauseValueCriteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CauseValue.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* O-CauseValueCriteria */
typedef struct O_CauseValueCriteria {
	A_SEQUENCE_OF(CauseValue_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} O_CauseValueCriteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_O_CauseValueCriteria;

#ifdef __cplusplus
}
#endif

#endif	/* _O_CauseValueCriteria_H_ */
