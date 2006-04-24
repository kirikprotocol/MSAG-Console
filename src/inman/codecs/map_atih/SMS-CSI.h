#ident "$Id$"

#ifndef	_SMS_CSI_H_
#define	_SMS_CSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CamelCapabilityHandling.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SMS_CAMEL_TDP_DataList;
struct ExtensionContainer;

/* SMS-CSI */
typedef struct SMS_CSI {
	struct SMS_CAMEL_TDP_DataList	*sms_CAMEL_TDP_DataList	/* OPTIONAL */;
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
} SMS_CSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMS_CSI;

/* Referred external types */
#include <SMS-CAMEL-TDP-DataList.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SMS_CSI_H_ */
