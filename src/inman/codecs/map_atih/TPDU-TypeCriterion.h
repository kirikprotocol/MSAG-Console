#ident "$Id$"

#ifndef	_TPDU_TypeCriterion_H_
#define	_TPDU_TypeCriterion_H_


#include <asn_application.h>

/* Including external dependencies */
#include <MT-SMS-TPDU-Type.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPDU-TypeCriterion */
typedef struct TPDU_TypeCriterion {
	A_SEQUENCE_OF(MT_SMS_TPDU_Type_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TPDU_TypeCriterion_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TPDU_TypeCriterion;

#ifdef __cplusplus
}
#endif

#endif	/* _TPDU_TypeCriterion_H_ */
