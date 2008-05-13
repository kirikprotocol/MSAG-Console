#ifndef	_UnknownSubscriberParam_H_
#define	_UnknownSubscriberParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include "UnknownSubscriberDiagnostic.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* UnknownSubscriberParam */
typedef struct UnknownSubscriberParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	UnknownSubscriberDiagnostic_t	*unknownSubscriberDiagnostic	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UnknownSubscriberParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnknownSubscriberParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _UnknownSubscriberParam_H_ */
