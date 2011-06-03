#ifndef	_ExtensibleSystemFailureParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ExtensibleSystemFailureParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NetworkResource.h>
#include <AdditionalNetworkResource.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ExtensibleSystemFailureParam */
typedef struct ExtensibleSystemFailureParam {
	NetworkResource_t	*networkResource	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AdditionalNetworkResource_t	*additionalNetworkResource	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ExtensibleSystemFailureParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ExtensibleSystemFailureParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ExtensibleSystemFailureParam_H_ */
