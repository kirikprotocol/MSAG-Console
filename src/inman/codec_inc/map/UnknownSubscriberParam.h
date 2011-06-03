#ifndef	_UnknownSubscriberParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_UnknownSubscriberParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <UnknownSubscriberDiagnostic.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* UnknownSubscriberParam */
typedef struct UnknownSubscriberParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	UnknownSubscriberDiagnostic_t	*unknownSubscriberDiagnostic	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UnknownSubscriberParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnknownSubscriberParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _UnknownSubscriberParam_H_ */
