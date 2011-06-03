#ifndef	_DestinationNumberCriteria_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DestinationNumberCriteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <MatchType.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct DestinationNumberList;
struct DestinationNumberLengthList;

/* DestinationNumberCriteria */
typedef struct DestinationNumberCriteria {
	MatchType_t	 matchType;
	struct DestinationNumberList	*destinationNumberList	/* OPTIONAL */;
	struct DestinationNumberLengthList	*destinationNumberLengthList	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DestinationNumberCriteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DestinationNumberCriteria;

/* Referred external types */
#include <DestinationNumberList.h>
#include <DestinationNumberLengthList.h>

#ifdef __cplusplus
}
#endif

#endif	/* _DestinationNumberCriteria_H_ */
