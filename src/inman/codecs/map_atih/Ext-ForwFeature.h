#ident "$Id$"

#ifndef	_Ext_ForwFeature_H_
#define	_Ext_ForwFeature_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-SS-Status.h>
#include <ISDN-AddressString.h>
#include <ISDN-SubaddressString.h>
#include <Ext-ForwOptions.h>
#include <Ext-NoRepCondTime.h>
#include <FTN-AddressString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Ext_BasicServiceCode;
struct ExtensionContainer;

/* Ext-ForwFeature */
typedef struct Ext_ForwFeature {
	struct Ext_BasicServiceCode	*basicService	/* OPTIONAL */;
	Ext_SS_Status_t	 ss_Status;
	ISDN_AddressString_t	*forwardedToNumber	/* OPTIONAL */;
	ISDN_SubaddressString_t	*forwardedToSubaddress	/* OPTIONAL */;
	Ext_ForwOptions_t	*forwardingOptions	/* OPTIONAL */;
	Ext_NoRepCondTime_t	*noReplyConditionTime	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	FTN_AddressString_t	*longForwardedToNumber	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Ext_ForwFeature_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_ForwFeature;

/* Referred external types */
#include <Ext-BasicServiceCode.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_ForwFeature_H_ */
