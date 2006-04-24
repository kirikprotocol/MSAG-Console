#ident "$Id$"

#ifndef	_LocationInformationGPRScap_H_
#define	_LocationInformationGPRScap_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <RAIdentity.h>
#include <GeographicalInformation.h>
#include <ISDN-AddressString.h>
#include <LSAIdentity.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* LocationInformationGPRScap */
typedef struct LocationInformationGPRScap {
	OCTET_STRING_t	*cellGlobalIdOrServiceAreaIdOrLAI	/* OPTIONAL */;
	RAIdentity_t	*routeingAreaIdentity	/* OPTIONAL */;
	GeographicalInformation_t	*geographicalInformation	/* OPTIONAL */;
	ISDN_AddressString_t	*sgsn_Number	/* OPTIONAL */;
	LSAIdentity_t	*selectedLSAIdentity	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*sai_Present	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInformationGPRScap_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInformationGPRScap;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _LocationInformationGPRScap_H_ */
