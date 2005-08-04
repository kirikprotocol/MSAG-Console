#ifndef	_ExtensionField_H_
#define	_ExtensionField_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Code.h>
#include <CriticalityType.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ExtensionField */
typedef struct ExtensionField {
	Code_t	 type;
	CriticalityType_t	*criticality	/* DEFAULT 0 */;
	OCTET_STRING_t	 value;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ExtensionField_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ExtensionField;

#ifdef __cplusplus
}
#endif

#endif	/* _ExtensionField_H_ */
