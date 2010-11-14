#ident "$Id$"

#ifndef	_SeqOfType1_H_
#define	_SeqOfType1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SeqType1;

/* SeqOfType1 */
typedef struct SeqOfType1 {
	A_SEQUENCE_OF(struct SeqType1) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqOfType1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqOfType1;

/* Referred external types */
#include <SeqType1.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SeqOfType1_H_ */
