#ident "$Id$"

#ifndef	_SeqTypeExplicit1_H_
#define	_SeqTypeExplicit1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SeqTypeExplicit1 */
typedef struct SeqTypeExplicit1 {
	long	 a;
	long	 b;
	NULL_t	 c;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SeqTypeExplicit1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SeqTypeExplicit1;

#ifdef __cplusplus
}
#endif

#endif	/* _SeqTypeExplicit1_H_ */
