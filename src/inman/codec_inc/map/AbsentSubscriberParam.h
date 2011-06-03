#ifndef	_AbsentSubscriberParam_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_AbsentSubscriberParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <AbsentSubscriberReason.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* AbsentSubscriberParam */
typedef struct AbsentSubscriberParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AbsentSubscriberReason_t	*absentSubscriberReason	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AbsentSubscriberParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AbsentSubscriberParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _AbsentSubscriberParam_H_ */
