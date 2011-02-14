#ident "$Id$"

#ifndef	_ChoiceType2_H_
#define	_ChoiceType2_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <BOOLEAN.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ChoiceType2_PR {
	ChoiceType2_PR_NOTHING,	/* No components present */
	ChoiceType2_PR_case1,
	ChoiceType2_PR_case2,
} ChoiceType2_PR;

/* ChoiceType2 */
typedef struct ChoiceType2 {
	ChoiceType2_PR present;
	union ChoiceType2_u {
		long	 case1;
		BOOLEAN_t	 case2;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChoiceType2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChoiceType2;

#ifdef __cplusplus
}
#endif

#endif	/* _ChoiceType2_H_ */
