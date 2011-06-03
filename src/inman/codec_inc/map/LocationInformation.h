#ifndef	_LocationInformation_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_LocationInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <AgeOfLocationInformation.h>
#include <GeographicalInformation.h>
#include <ISDN-AddressString.h>
#include <LocationNumber.h>
#include <LSAIdentity.h>
#include <GeodeticInformation.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CellGlobalIdOrServiceAreaIdOrLAI;
struct ExtensionContainer;

/* LocationInformation */
typedef struct LocationInformation {
	AgeOfLocationInformation_t	*ageOfLocationInformation	/* OPTIONAL */;
	GeographicalInformation_t	*geographicalInformation	/* OPTIONAL */;
	ISDN_AddressString_t	*vlr_number	/* OPTIONAL */;
	LocationNumber_t	*locationNumber	/* OPTIONAL */;
	struct CellGlobalIdOrServiceAreaIdOrLAI	*cellGlobalIdOrServiceAreaIdOrLAI	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	LSAIdentity_t	*selectedLSA_Id	/* OPTIONAL */;
	ISDN_AddressString_t	*msc_Number	/* OPTIONAL */;
	GeodeticInformation_t	*geodeticInformation	/* OPTIONAL */;
	NULL_t	*currentLocationRetrieved	/* OPTIONAL */;
	NULL_t	*sai_Present	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInformation;

/* Referred external types */
#include <CellGlobalIdOrServiceAreaIdOrLAI.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _LocationInformation_H_ */
