#ifndef	_ATSI_NotAllowedParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ATSI_NotAllowedParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ATSI-NotAllowedParam */
typedef struct ATSI_NotAllowedParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ATSI_NotAllowedParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ATSI_NotAllowedParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ATSI_NotAllowedParam_H_ */
