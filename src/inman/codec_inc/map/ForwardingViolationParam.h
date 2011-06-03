#ifndef	_ForwardingViolationParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ForwardingViolationParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ForwardingViolationParam */
typedef struct ForwardingViolationParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ForwardingViolationParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingViolationParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingViolationParam_H_ */
