#ifndef	_T_BCSM_CAMEL_TDP_Criteria_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_T_BCSM_CAMEL_TDP_Criteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <T-BcsmTriggerDetectionPoint.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct BasicServiceCriteria;
struct T_CauseValueCriteria;

/* T-BCSM-CAMEL-TDP-Criteria */
typedef struct T_BCSM_CAMEL_TDP_Criteria {
	T_BcsmTriggerDetectionPoint_t	 t_BCSM_TriggerDetectionPoint;
	struct BasicServiceCriteria	*basicServiceCriteria	/* OPTIONAL */;
	struct T_CauseValueCriteria	*t_CauseValueCriteria	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} T_BCSM_CAMEL_TDP_Criteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_BCSM_CAMEL_TDP_Criteria;

/* Referred external types */
#include <BasicServiceCriteria.h>
#include <T-CauseValueCriteria.h>

#ifdef __cplusplus
}
#endif

#endif	/* _T_BCSM_CAMEL_TDP_Criteria_H_ */
