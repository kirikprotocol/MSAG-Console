#ifndef	_DestinationNumberLengthList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DestinationNumberLengthList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DestinationNumberLengthList */
typedef struct DestinationNumberLengthList {
	A_SEQUENCE_OF(long) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DestinationNumberLengthList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DestinationNumberLengthList;

#ifdef __cplusplus
}
#endif

#endif	/* _DestinationNumberLengthList_H_ */
