#ident "$Id$"

#ifndef	_SeqType2_H_
#define	_SeqType2_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqType2 */
typedef struct SeqType2 {
	long	 a;
	long	*b	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqType2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqType2;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqType2_H_ */
