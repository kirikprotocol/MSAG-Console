#ident "$Id$"

#ifndef	_CallForwardingData_H_
#define	_CallForwardingData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Ext-ForwFeatureList.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CallForwardingData */
typedef struct CallForwardingData {
	Ext_ForwFeatureList_t	 forwardingFeatureList;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CallForwardingData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallForwardingData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CallForwardingData_H_ */
