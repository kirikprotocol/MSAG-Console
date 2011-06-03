#ifndef	_DP_AnalysedInfoCriteriaList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DP_AnalysedInfoCriteriaList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct DP_AnalysedInfoCriterium;

/* DP-AnalysedInfoCriteriaList */
typedef struct DP_AnalysedInfoCriteriaList {
	A_SEQUENCE_OF(struct DP_AnalysedInfoCriterium) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DP_AnalysedInfoCriteriaList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DP_AnalysedInfoCriteriaList;

/* Referred external types */
#include <DP-AnalysedInfoCriterium.h>

#ifdef __cplusplus
}
#endif

#endif	/* _DP_AnalysedInfoCriteriaList_H_ */
