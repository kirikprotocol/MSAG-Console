#ifndef	_NAEA_PreferredCI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_NAEA_PreferredCI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NAEA-CIC.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* NAEA-PreferredCI */
typedef struct NAEA_PreferredCI {
	NAEA_CIC_t	 naea_PreferredCIC;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NAEA_PreferredCI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NAEA_PreferredCI;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _NAEA_PreferredCI_H_ */
