#ifndef	_O_BcsmCamelTDPCriteriaList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_O_BcsmCamelTDPCriteriaList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct O_BcsmCamelTDP_Criteria;

/* O-BcsmCamelTDPCriteriaList */
typedef struct O_BcsmCamelTDPCriteriaList {
	A_SEQUENCE_OF(struct O_BcsmCamelTDP_Criteria) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} O_BcsmCamelTDPCriteriaList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_O_BcsmCamelTDPCriteriaList;

/* Referred external types */
#include <O-BcsmCamelTDP-Criteria.h>

#ifdef __cplusplus
}
#endif

#endif	/* _O_BcsmCamelTDPCriteriaList_H_ */
