#ifndef	_O_BcsmCamelTDP_Criteria_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_O_BcsmCamelTDP_Criteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <O-BcsmTriggerDetectionPoint.h>
#include <CallTypeCriteria.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct DestinationNumberCriteria;
struct BasicServiceCriteria;
struct O_CauseValueCriteria;
struct ExtensionContainer;

/* O-BcsmCamelTDP-Criteria */
typedef struct O_BcsmCamelTDP_Criteria {
	O_BcsmTriggerDetectionPoint_t	 o_BcsmTriggerDetectionPoint;
	struct DestinationNumberCriteria	*destinationNumberCriteria	/* OPTIONAL */;
	struct BasicServiceCriteria	*basicServiceCriteria	/* OPTIONAL */;
	CallTypeCriteria_t	*callTypeCriteria	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct O_CauseValueCriteria	*o_CauseValueCriteria	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} O_BcsmCamelTDP_Criteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_O_BcsmCamelTDP_Criteria;

/* Referred external types */
#include <DestinationNumberCriteria.h>
#include <BasicServiceCriteria.h>
#include <O-CauseValueCriteria.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _O_BcsmCamelTDP_Criteria_H_ */
