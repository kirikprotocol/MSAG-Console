#ifndef	_CCBS_Indicators_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CCBS_Indicators_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CCBS-Indicators */
typedef struct CCBS_Indicators {
	NULL_t	*ccbs_Possible	/* OPTIONAL */;
	NULL_t	*keepCCBS_CallIndicator	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CCBS_Indicators_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CCBS_Indicators;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CCBS_Indicators_H_ */
