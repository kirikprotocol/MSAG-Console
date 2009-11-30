#ifndef	_LocationInformationGPRS_H_
#define	_LocationInformationGPRS_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RAIdentity.h"
#include "GeographicalInformation.h"
#include "ISDN-AddressString.h"
#include "LSAIdentity.h"
#include <NULL.h>
#include "GeodeticInformation.h"
#include "AgeOfLocationInformation.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CellGlobalIdOrServiceAreaIdOrLAI;
struct ExtensionContainer;

/* LocationInformationGPRS */
typedef struct LocationInformationGPRS {
	struct CellGlobalIdOrServiceAreaIdOrLAI	*cellGlobalIdOrServiceAreaIdOrLAI	/* OPTIONAL */;
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
	GeodeticInformation_t	*geodeticInformation	/* OPTIONAL */;
	NULL_t	*currentLocationRetrieved	/* OPTIONAL */;
	AgeOfLocationInformation_t	*ageOfLocationInformation	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInformationGPRS_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInformationGPRS;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CellGlobalIdOrServiceAreaIdOrLAI.h"
#include "ExtensionContainer.h"

#endif	/* _LocationInformationGPRS_H_ */
