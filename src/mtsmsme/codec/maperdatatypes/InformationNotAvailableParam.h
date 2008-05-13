#ifndef	_InformationNotAvailableParam_H_
#define	_InformationNotAvailableParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* InformationNotAvailableParam */
typedef struct InformationNotAvailableParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InformationNotAvailableParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InformationNotAvailableParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _InformationNotAvailableParam_H_ */
