#ifndef	_T_BcsmCamelTDPData_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_T_BcsmCamelTDPData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <T-BcsmTriggerDetectionPoint.h>
#include <ServiceKey.h>
#include <ISDN-AddressString.h>
#include <DefaultCallHandling.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* T-BcsmCamelTDPData */
typedef struct T_BcsmCamelTDPData {
	T_BcsmTriggerDetectionPoint_t	 t_BcsmTriggerDetectionPoint;
	ServiceKey_t	 serviceKey;
	ISDN_AddressString_t	 gsmSCF_Address;
	DefaultCallHandling_t	 defaultCallHandling;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} T_BcsmCamelTDPData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_BcsmCamelTDPData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _T_BcsmCamelTDPData_H_ */
