#ifndef	_DestinationNumberList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DestinationNumberList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ISDN-AddressString.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DestinationNumberList */
typedef struct DestinationNumberList {
	A_SEQUENCE_OF(ISDN_AddressString_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DestinationNumberList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DestinationNumberList;

#ifdef __cplusplus
}
#endif

#endif	/* _DestinationNumberList_H_ */
