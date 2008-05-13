#ifndef	_UnidentifiedSubParam_H_
#define	_UnidentifiedSubParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* UnidentifiedSubParam */
typedef struct UnidentifiedSubParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UnidentifiedSubParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnidentifiedSubParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _UnidentifiedSubParam_H_ */
