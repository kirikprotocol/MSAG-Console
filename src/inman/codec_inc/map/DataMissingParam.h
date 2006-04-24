#ident "$Id$"

#ifndef	_DataMissingParam_H_
#define	_DataMissingParam_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* DataMissingParam */
typedef struct DataMissingParam {
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DataMissingParam_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DataMissingParam;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _DataMissingParam_H_ */
