#ifndef	_Additional_Number_H_
#define	_Additional_Number_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Additional_Number_PR {
	Additional_Number_PR_NOTHING,	/* No components present */
	Additional_Number_PR_msc_Number,
	Additional_Number_PR_sgsn_Number
} Additional_Number_PR;

/* Additional-Number */
typedef struct Additional_Number {
	Additional_Number_PR present;
	union Additional_Number_u {
		ISDN_AddressString_t	 msc_Number;
		ISDN_AddressString_t	 sgsn_Number;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Additional_Number_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Additional_Number;

#ifdef __cplusplus
}
#endif

#endif	/* _Additional_Number_H_ */
