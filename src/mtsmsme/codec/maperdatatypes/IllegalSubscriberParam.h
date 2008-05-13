#ifndef	_IllegalSubscriberParam_H_
#define	_IllegalSubscriberParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* IllegalSubscriberParam */
typedef struct IllegalSubscriberParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IllegalSubscriberParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IllegalSubscriberParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _IllegalSubscriberParam_H_ */
