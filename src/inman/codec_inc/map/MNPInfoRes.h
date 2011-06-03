#ifndef	_MNPInfoRes_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_MNPInfoRes_H_


#include <asn_application.h>

/* Including external dependencies */
#include <RouteingNumber.h>
#include <IMSI.h>
#include <ISDN-AddressString.h>
#include <NumberPortabilityStatus.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* MNPInfoRes */
typedef struct MNPInfoRes {
	RouteingNumber_t	*routeingNumber	/* OPTIONAL */;
	IMSI_t	*imsi	/* OPTIONAL */;
	ISDN_AddressString_t	*msisdn	/* OPTIONAL */;
	NumberPortabilityStatus_t	*numberPortabilityStatus	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MNPInfoRes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MNPInfoRes;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _MNPInfoRes_H_ */
