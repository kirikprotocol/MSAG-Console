#ifndef	_CancelLocationArg_H_
#define	_CancelLocationArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "Identity.h"
#include "CancellationType.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CancelLocationArg */
typedef struct CancelLocationArg {
	Identity_t	 identity;
	CancellationType_t	*cancellationType	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CancelLocationArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CancelLocationArg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _CancelLocationArg_H_ */
