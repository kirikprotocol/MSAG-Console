#ident "$Id$"

#ifndef	_SS_CamelData_H_
#define	_SS_CamelData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SS-EventList.h>
#include <ISDN-AddressString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* SS-CamelData */
typedef struct SS_CamelData {
	SS_EventList_t	 ss_EventList;
	ISDN_AddressString_t	 gsmSCF_Address;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SS_CamelData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SS_CamelData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SS_CamelData_H_ */
