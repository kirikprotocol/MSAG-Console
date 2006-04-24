#ident "$Id$"

#ifndef	_ODB_Info_H_
#define	_ODB_Info_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ODB-Data.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ODB-Info */
typedef struct ODB_Info {
	ODB_Data_t	 odb_Data;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ODB_Info_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ODB_Info;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ODB_Info_H_ */
