#ifndef	_ExtensionContainer_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ExtensionContainer_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PrivateExtensionList;
struct PCS_Extensions;

/* ExtensionContainer */
typedef struct ExtensionContainer {
	struct PrivateExtensionList	*privateExtensionList	/* OPTIONAL */;
	struct PCS_Extensions	*pcs_Extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ExtensionContainer_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ExtensionContainer;

/* Referred external types */
#include <PrivateExtensionList.h>
#include <PCS-Extensions.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ExtensionContainer_H_ */
