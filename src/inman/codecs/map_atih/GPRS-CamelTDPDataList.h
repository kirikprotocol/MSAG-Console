#ident "$Id$"

#ifndef	_GPRS_CamelTDPDataList_H_
#define	_GPRS_CamelTDPDataList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct GPRS_CamelTDPData;

/* GPRS-CamelTDPDataList */
typedef struct GPRS_CamelTDPDataList {
	A_SEQUENCE_OF(struct GPRS_CamelTDPData) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GPRS_CamelTDPDataList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRS_CamelTDPDataList;

/* Referred external types */
#include <GPRS-CamelTDPData.h>

#ifdef __cplusplus
}
#endif

#endif	/* _GPRS_CamelTDPDataList_H_ */
