#ident "$Id$"

#ifndef	_GPRS_CSI_H_
#define	_GPRS_CSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CamelCapabilityHandling.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct GPRS_CamelTDPDataList;
struct ExtensionContainer;

/* GPRS-CSI */
typedef struct GPRS_CSI {
	struct GPRS_CamelTDPDataList	*gprs_CamelTDPDataList	/* OPTIONAL */;
	CamelCapabilityHandling_t	*camelCapabilityHandling	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	NULL_t	*csi_Active	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GPRS_CSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRS_CSI;

/* Referred external types */
#include <GPRS-CamelTDPDataList.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _GPRS_CSI_H_ */
