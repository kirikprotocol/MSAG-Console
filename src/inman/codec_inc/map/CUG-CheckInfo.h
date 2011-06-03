#ifndef	_CUG_CheckInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CUG_CheckInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CUG-Interlock.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* CUG-CheckInfo */
typedef struct CUG_CheckInfo {
	CUG_Interlock_t	 cug_Interlock;
	NULL_t	*cug_OutgoingAccess	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CUG_CheckInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CUG_CheckInfo;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CUG_CheckInfo_H_ */
