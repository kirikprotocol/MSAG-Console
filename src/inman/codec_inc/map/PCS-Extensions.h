#ifndef	_PCS_Extensions_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PCS_Extensions_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PCS-Extensions */
typedef struct PCS_Extensions {
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PCS_Extensions_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PCS_Extensions;

#ifdef __cplusplus
}
#endif

#endif	/* _PCS_Extensions_H_ */
