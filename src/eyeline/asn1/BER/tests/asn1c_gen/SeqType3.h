#ident "$Id$"

#ifndef	_SeqType3_H_
#define	_SeqType3_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqType3 */
typedef struct SeqType3 {
	long	 a;
	long	*b	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqType3_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqType3;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqType3_H_ */
