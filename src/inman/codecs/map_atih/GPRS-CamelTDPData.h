#ident "$Id$"

#ifndef	_GPRS_CamelTDPData_H_
#define	_GPRS_CamelTDPData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <GPRS-TriggerDetectionPoint.h>
#include <ServiceKey.h>
#include <ISDN-AddressString.h>
#include <DefaultGPRS-Handling.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* GPRS-CamelTDPData */
typedef struct GPRS_CamelTDPData {
	GPRS_TriggerDetectionPoint_t	 gprs_TriggerDetectionPoint;
	ServiceKey_t	 serviceKey;
	ISDN_AddressString_t	 gsmSCF_Address;
	DefaultGPRS_Handling_t	 defaultSessionHandling;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GPRS_CamelTDPData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRS_CamelTDPData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _GPRS_CamelTDPData_H_ */
