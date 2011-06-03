#ifndef	_ExtensibleCallBarredParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ExtensibleCallBarredParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CallBarringCause.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ExtensibleCallBarredParam */
typedef struct ExtensibleCallBarredParam {
	CallBarringCause_t	*callBarringCause	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*unauthorisedMessageOriginator	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ExtensibleCallBarredParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ExtensibleCallBarredParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ExtensibleCallBarredParam_H_ */
