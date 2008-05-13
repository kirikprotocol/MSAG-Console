#ifndef	_AlertServiceCentreArg_H_
#define	_AlertServiceCentreArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "AddressString.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AlertServiceCentreArg */
typedef struct AlertServiceCentreArg {
	ISDN_AddressString_t	 msisdn;
	AddressString_t	 serviceCentreAddress;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AlertServiceCentreArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AlertServiceCentreArg;

#ifdef __cplusplus
}
#endif

#endif	/* _AlertServiceCentreArg_H_ */
