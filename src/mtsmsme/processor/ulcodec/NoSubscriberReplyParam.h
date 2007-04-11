#ifndef	_NoSubscriberReplyParam_H_
#define	_NoSubscriberReplyParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* NoSubscriberReplyParam */
typedef struct NoSubscriberReplyParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NoSubscriberReplyParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NoSubscriberReplyParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _NoSubscriberReplyParam_H_ */
