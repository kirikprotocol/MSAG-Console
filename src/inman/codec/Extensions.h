#ifndef	_Extensions_H_
#define	_Extensions_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionField;

/* Extensions */
typedef struct Extensions {
	A_SEQUENCE_OF(struct ExtensionField) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Extensions_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Extensions;

/* Referred external types */
#include <ExtensionField.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Extensions_H_ */
