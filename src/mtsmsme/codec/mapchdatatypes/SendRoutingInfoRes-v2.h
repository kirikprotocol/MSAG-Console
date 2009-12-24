#ifndef	_SendRoutingInfoRes_v2_H_
#define	_SendRoutingInfoRes_v2_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "RoutingInfo.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CUG_CheckInfo;

/* SendRoutingInfoRes-v2 */
typedef struct SendRoutingInfoRes_v2 {
	IMSI_t	 imsi;
	RoutingInfo_t	 routingInfo;
	struct CUG_CheckInfo	*cug_CheckInfo	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SendRoutingInfoRes_v2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoRes_v2;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CUG-CheckInfo.h"

#endif	/* _SendRoutingInfoRes_v2_H_ */
