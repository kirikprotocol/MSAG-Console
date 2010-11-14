#ident "$Id$"

#ifndef	_SeqType4_H_
#define	_SeqType4_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqType4 */
typedef struct SeqType4 {
	long	 a;
	long	*b	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	long	*c	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqType4_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqType4;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqType4_H_ */
