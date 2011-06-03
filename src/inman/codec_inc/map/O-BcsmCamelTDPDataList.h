#ifndef	_O_BcsmCamelTDPDataList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_O_BcsmCamelTDPDataList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct O_BcsmCamelTDPData;

/* O-BcsmCamelTDPDataList */
typedef struct O_BcsmCamelTDPDataList {
	A_SEQUENCE_OF(struct O_BcsmCamelTDPData) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} O_BcsmCamelTDPDataList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_O_BcsmCamelTDPDataList;

/* Referred external types */
#include <O-BcsmCamelTDPData.h>

#ifdef __cplusplus
}
#endif

#endif	/* _O_BcsmCamelTDPDataList_H_ */
