#ident "$Id$"

#ifndef	_SeqType5_H_
#define	_SeqType5_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ChoiceType1.h>
#include <ChoiceType2.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqType5 */
typedef struct SeqType5 {
	ChoiceType1_t	 a;
	ChoiceType2_t	 b;
	ChoiceType1_t	 c;
	ChoiceType2_t	 d;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqType5_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqType5;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqType5_H_ */
