#ifndef	_SS_ForBS_Code_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SS_ForBS_Code_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SS-Code.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct BasicServiceCode;

/* SS-ForBS-Code */
typedef struct SS_ForBS_Code {
	SS_Code_t	 ss_Code;
	struct BasicServiceCode	*basicService	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*longFTN_Supported	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SS_ForBS_Code_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SS_ForBS_Code;

/* Referred external types */
#include <BasicServiceCode.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SS_ForBS_Code_H_ */
