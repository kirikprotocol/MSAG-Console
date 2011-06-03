#ifndef	_PrivateExtensionList_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PrivateExtensionList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct PrivateExtension;

/* PrivateExtensionList */
typedef struct PrivateExtensionList {
	A_SEQUENCE_OF(struct PrivateExtension) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PrivateExtensionList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PrivateExtensionList;

/* Referred external types */
#include <PrivateExtension.h>

#ifdef __cplusplus
}
#endif

#endif	/* _PrivateExtensionList_H_ */
