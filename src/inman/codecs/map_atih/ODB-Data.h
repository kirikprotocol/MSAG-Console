#ident "$Id$"

#ifndef	_ODB_Data_H_
#define	_ODB_Data_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ODB-GeneralData.h>
#include <ODB-HPLMN-Data.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ODB-Data */
typedef struct ODB_Data {
	ODB_GeneralData_t	 odb_GeneralData;
	ODB_HPLMN_Data_t	*odb_HPLMN_Data	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ODB_Data_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ODB_Data;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ODB_Data_H_ */
