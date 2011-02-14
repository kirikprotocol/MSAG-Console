#ident "$Id$"

#ifndef	_ChoiceType1_H_
#define	_ChoiceType1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <BOOLEAN.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ChoiceType1_PR {
	ChoiceType1_PR_NOTHING,	/* No components present */
	ChoiceType1_PR_case1,
	ChoiceType1_PR_case2,
} ChoiceType1_PR;

/* ChoiceType1 */
typedef struct ChoiceType1 {
	ChoiceType1_PR present;
	union ChoiceType1_u {
		long	 case1;
		BOOLEAN_t	 case2;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChoiceType1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChoiceType1;

#ifdef __cplusplus
}
#endif

#endif	/* _ChoiceType1_H_ */
