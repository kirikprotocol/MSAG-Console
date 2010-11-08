#ident "$Id$"

#ifndef	_SeqType1_H_
#define	_SeqType1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqType1 */
typedef struct SeqType1 {
	long	 a;
	long	 b;
	NULL_t	 c;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqType1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqType1;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqType1_H_ */
