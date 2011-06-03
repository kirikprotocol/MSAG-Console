#ifndef	_SS_NotAvailableParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SS_NotAvailableParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* SS-NotAvailableParam */
typedef struct SS_NotAvailableParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SS_NotAvailableParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SS_NotAvailableParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SS_NotAvailableParam_H_ */
