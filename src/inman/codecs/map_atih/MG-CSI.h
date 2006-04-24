#ident "$Id$"

#ifndef	_MG_CSI_H_
#define	_MG_CSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <MobilityTriggers.h>
#include <ServiceKey.h>
#include <ISDN-AddressString.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* MG-CSI */
typedef struct MG_CSI {
	MobilityTriggers_t	 mobilityTriggers;
	ServiceKey_t	 serviceKey;
	ISDN_AddressString_t	 gsmSCF_Address;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	NULL_t	*csi_Active	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MG_CSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MG_CSI;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _MG_CSI_H_ */
