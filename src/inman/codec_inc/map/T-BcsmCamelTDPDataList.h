#ifndef	_T_BcsmCamelTDPDataList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_T_BcsmCamelTDPDataList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct T_BcsmCamelTDPData;

/* T-BcsmCamelTDPDataList */
typedef struct T_BcsmCamelTDPDataList {
	A_SEQUENCE_OF(struct T_BcsmCamelTDPData) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} T_BcsmCamelTDPDataList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_BcsmCamelTDPDataList;

/* Referred external types */
#include <T-BcsmCamelTDPData.h>

#ifdef __cplusplus
}
#endif

#endif	/* _T_BcsmCamelTDPDataList_H_ */
