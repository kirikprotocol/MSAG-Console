#ident "$Id$"

#ifndef	_SS_EventList_H_
#define	_SS_EventList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SS-Code.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SS-EventList */
typedef struct SS_EventList {
	A_SEQUENCE_OF(SS_Code_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SS_EventList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SS_EventList;

#ifdef __cplusplus
}
#endif

#endif	/* _SS_EventList_H_ */
