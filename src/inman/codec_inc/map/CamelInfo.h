#ifndef	_CamelInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CamelInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SupportedCamelPhases.h>
#include <NULL.h>
#include <OfferedCamel4CSIs.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CamelInfo */
typedef struct CamelInfo {
	SupportedCamelPhases_t	 supportedCamelPhases;
	NULL_t	*suppress_T_CSI	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	OfferedCamel4CSIs_t	*offeredCamel4CSIs	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CamelInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CamelInfo;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CamelInfo_H_ */
