#ifndef	_FacilityNotSupParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_FacilityNotSupParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* FacilityNotSupParam */
typedef struct FacilityNotSupParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*shapeOfLocationEstimateNotSupported	/* OPTIONAL */;
	NULL_t	*neededLcsCapabilityNotSupportedInServingNode	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} FacilityNotSupParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_FacilityNotSupParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _FacilityNotSupParam_H_ */
