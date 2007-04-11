#ifndef	_BusySubscriberParam_H_
#define	_BusySubscriberParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* BusySubscriberParam */
typedef struct BusySubscriberParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*ccbs_Possible	/* OPTIONAL */;
	NULL_t	*ccbs_Busy	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BusySubscriberParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BusySubscriberParam;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _BusySubscriberParam_H_ */
