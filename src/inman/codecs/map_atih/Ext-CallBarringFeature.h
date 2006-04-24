#ident "$Id$"

#ifndef	_Ext_CallBarringFeature_H_
#define	_Ext_CallBarringFeature_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-SS-Status.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ext_BasicServiceCode;
struct ExtensionContainer;

/* Ext-CallBarringFeature */
typedef struct Ext_CallBarringFeature {
	struct Ext_BasicServiceCode	*basicService	/* OPTIONAL */;
	Ext_SS_Status_t	 ss_Status;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_CallBarringFeature_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_CallBarringFeature;

/* Referred external types */
#include <Ext-BasicServiceCode.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_CallBarringFeature_H_ */
