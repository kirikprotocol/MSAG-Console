#ident "$Id$"

#ifndef	_CAMEL_FCISMSBillingChargingCharacteristics_H_
#define	_CAMEL_FCISMSBillingChargingCharacteristics_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <AppendFreeFormatData.h>
#include <constr_SEQUENCE.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CAMEL_FCISMSBillingChargingCharacteristics_PR {
	CAMEL_FCISMSBillingChargingCharacteristics_PR_NOTHING,	/* No components present */
	CAMEL_FCISMSBillingChargingCharacteristics_PR_fCIBCCCAMELsequence1
} CAMEL_FCISMSBillingChargingCharacteristics_PR;

typedef struct fCIBCCCAMELsequence1_s {
	OCTET_STRING_t	 freeFormatData;
	AppendFreeFormatData_t	*appendFreeFormatData	/* DEFAULT 0 */;

	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} fCIBCCCAMELsequence1_t;

/* CAMEL-FCISMSBillingChargingCharacteristics */
typedef struct CAMEL_FCISMSBillingChargingCharacteristics {
	CAMEL_FCISMSBillingChargingCharacteristics_PR present;
	union CAMEL_FCISMSBillingChargingCharacteristics_u {
		struct fCIBCCCAMELsequence1_s  fCIBCCCAMELsequence1;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CAMEL_FCISMSBillingChargingCharacteristics_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics;

#ifdef __cplusplus
}
#endif

#endif	/* _CAMEL_FCISMSBillingChargingCharacteristics_H_ */
