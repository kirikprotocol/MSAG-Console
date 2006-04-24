#ident "$Id$"

#ifndef	_MT_smsCAMELTDP_CriteriaList_H_
#define	_MT_smsCAMELTDP_CriteriaList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MT_smsCAMELTDP_Criteria;

/* MT-smsCAMELTDP-CriteriaList */
typedef struct MT_smsCAMELTDP_CriteriaList {
	A_SEQUENCE_OF(struct MT_smsCAMELTDP_Criteria) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MT_smsCAMELTDP_CriteriaList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_smsCAMELTDP_CriteriaList;

/* Referred external types */
#include <MT-smsCAMELTDP-Criteria.h>

#ifdef __cplusplus
}
#endif

#endif	/* _MT_smsCAMELTDP_CriteriaList_H_ */
