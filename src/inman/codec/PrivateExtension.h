#ifndef	_PrivateExtension_H_
#define	_PrivateExtension_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OBJECT_IDENTIFIER.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PrivateExtension */
typedef struct PrivateExtension {
	OBJECT_IDENTIFIER_t	 extId;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PrivateExtension_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PrivateExtension;

#ifdef __cplusplus
}
#endif

#endif	/* _PrivateExtension_H_ */
