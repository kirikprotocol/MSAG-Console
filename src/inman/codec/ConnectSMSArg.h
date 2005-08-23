#ifndef	_ConnectSMSArg_H_
#define	_ConnectSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SMS-AddressString.h>
#include <CalledPartyBCDNumber.h>
#include <ISDN-AddressString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Extensions;

/* ConnectSMSArg */
typedef struct ConnectSMSArg {
	SMS_AddressString_t	*callingPartysNumber	/* OPTIONAL */;
	CalledPartyBCDNumber_t	*destinationSubscriberNumber	/* OPTIONAL */;
	ISDN_AddressString_t	*sMSCAddress	/* OPTIONAL */;
	struct Extensions	*extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ConnectSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ConnectSMSArg;

/* Referred external types */
#include <Extensions.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ConnectSMSArg_H_ */
