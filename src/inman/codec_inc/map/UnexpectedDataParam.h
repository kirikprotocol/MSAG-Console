#ifndef	_UnexpectedDataParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_UnexpectedDataParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* UnexpectedDataParam */
typedef struct UnexpectedDataParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UnexpectedDataParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnexpectedDataParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _UnexpectedDataParam_H_ */
