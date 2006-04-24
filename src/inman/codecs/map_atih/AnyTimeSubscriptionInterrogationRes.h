#ident "$Id$"

#ifndef	_AnyTimeSubscriptionInterrogationRes_H_
#define	_AnyTimeSubscriptionInterrogationRes_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SupportedCamelPhases.h>
#include <OfferedCamel4CSIs.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CallForwardingData;
struct CallBarringData;
struct ODB_Info;
struct CAMEL_SubscriptionInfo;
struct ExtensionContainer;

/* AnyTimeSubscriptionInterrogationRes */
typedef struct AnyTimeSubscriptionInterrogationRes {
	struct CallForwardingData	*callForwardingData	/* OPTIONAL */;
	struct CallBarringData	*callBarringData	/* OPTIONAL */;
	struct ODB_Info	*odb_Info	/* OPTIONAL */;
	struct CAMEL_SubscriptionInfo	*camel_SubscriptionInfo	/* OPTIONAL */;
	SupportedCamelPhases_t	*supportedVLR_CAMEL_Phases	/* OPTIONAL */;
	SupportedCamelPhases_t	*supportedSGSN_CAMEL_Phases	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	OfferedCamel4CSIs_t	*offeredCamel4CSIsInVLR	/* OPTIONAL */;
	OfferedCamel4CSIs_t	*offeredCamel4CSIsInSGSN	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AnyTimeSubscriptionInterrogationRes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AnyTimeSubscriptionInterrogationRes;

/* Referred external types */
#include <CallForwardingData.h>
#include <CallBarringData.h>
#include <ODB-Info.h>
#include <CAMEL-SubscriptionInfo.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _AnyTimeSubscriptionInterrogationRes_H_ */
