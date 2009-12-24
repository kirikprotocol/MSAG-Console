#ifndef	_SendRoutingInfoArg_v2_H_
#define	_SendRoutingInfoArg_v2_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "NumberOfForwarding.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CUG_CheckInfo;
struct ExternalSignalInfo;

/* SendRoutingInfoArg-v2 */
typedef struct SendRoutingInfoArg_v2 {
	ISDN_AddressString_t	 msisdn;
	struct CUG_CheckInfo	*cug_CheckInfo	/* OPTIONAL */;
	NumberOfForwarding_t	*numberOfForwarding	/* OPTIONAL */;
	struct ExternalSignalInfo	*networkSignalInfo	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SendRoutingInfoArg_v2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoArg_v2;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CUG-CheckInfo.h"
#include "ExternalSignalInfo.h"

#endif	/* _SendRoutingInfoArg_v2_H_ */
