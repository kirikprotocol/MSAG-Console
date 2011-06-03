#ifndef	_BasicServiceCriteria_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_BasicServiceCriteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ext_BasicServiceCode;

/* BasicServiceCriteria */
typedef struct BasicServiceCriteria {
	A_SEQUENCE_OF(struct Ext_BasicServiceCode) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BasicServiceCriteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BasicServiceCriteria;

/* Referred external types */
#include <Ext-BasicServiceCode.h>

#ifdef __cplusplus
}
#endif

#endif	/* _BasicServiceCriteria_H_ */
