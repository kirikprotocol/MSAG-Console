#ident "$Id$"

#ifndef	_MT_smsCAMELTDP_Criteria_H_
#define	_MT_smsCAMELTDP_Criteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SMS-TriggerDetectionPoint.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct TPDU_TypeCriterion;

/* MT-smsCAMELTDP-Criteria */
typedef struct MT_smsCAMELTDP_Criteria {
	SMS_TriggerDetectionPoint_t	 sms_TriggerDetectionPoint;
	struct TPDU_TypeCriterion	*tpdu_TypeCriterion	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MT_smsCAMELTDP_Criteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_smsCAMELTDP_Criteria;

/* Referred external types */
#include <TPDU-TypeCriterion.h>

#ifdef __cplusplus
}
#endif

#endif	/* _MT_smsCAMELTDP_Criteria_H_ */
