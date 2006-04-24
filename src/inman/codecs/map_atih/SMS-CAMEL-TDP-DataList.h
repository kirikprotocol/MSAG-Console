#ident "$Id$"

#ifndef	_SMS_CAMEL_TDP_DataList_H_
#define	_SMS_CAMEL_TDP_DataList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SMS_CAMEL_TDP_Data;

/* SMS-CAMEL-TDP-DataList */
typedef struct SMS_CAMEL_TDP_DataList {
	A_SEQUENCE_OF(struct SMS_CAMEL_TDP_Data) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SMS_CAMEL_TDP_DataList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMS_CAMEL_TDP_DataList;

/* Referred external types */
#include <SMS-CAMEL-TDP-Data.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SMS_CAMEL_TDP_DataList_H_ */
