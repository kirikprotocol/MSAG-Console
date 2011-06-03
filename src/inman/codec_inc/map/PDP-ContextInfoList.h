#ifndef	_PDP_ContextInfoList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PDP_ContextInfoList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PDP_ContextInfo;

/* PDP-ContextInfoList */
typedef struct PDP_ContextInfoList {
	A_SEQUENCE_OF(struct PDP_ContextInfo) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PDP_ContextInfoList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PDP_ContextInfoList;

/* Referred external types */
#include <PDP-ContextInfo.h>

#ifdef __cplusplus
}
#endif

#endif	/* _PDP_ContextInfoList_H_ */
