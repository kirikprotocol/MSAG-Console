#ident "$Id$"

#ifndef	_SMS_CAMEL_TDP_Data_H_
#define	_SMS_CAMEL_TDP_Data_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SMS-TriggerDetectionPoint.h>
#include <ServiceKey.h>
#include <ISDN-AddressString.h>
#include <DefaultSMS-Handling.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* SMS-CAMEL-TDP-Data */
typedef struct SMS_CAMEL_TDP_Data {
	SMS_TriggerDetectionPoint_t	 sms_TriggerDetectionPoint;
	ServiceKey_t	 serviceKey;
	ISDN_AddressString_t	 gsmSCF_Address;
	DefaultSMS_Handling_t	 defaultSMS_Handling;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SMS_CAMEL_TDP_Data_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMS_CAMEL_TDP_Data;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SMS_CAMEL_TDP_Data_H_ */
