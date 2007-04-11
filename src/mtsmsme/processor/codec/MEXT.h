#ifndef	_MEXT_H_
#define	_MEXT_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OBJECT_IDENTIFIER.h>
#include <NativeInteger.h>
#include <ObjectDescriptor.h>
#include <ANY.h>
#include <OCTET_STRING.h>
#include <BIT_STRING.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum mencoding_PR {
	mencoding_PR_NOTHING,	/* No components present */
	mencoding_PR_single_ASN1_type,
	mencoding_PR_octet_aligned,
	mencoding_PR_arbitrary
} mencoding_PR;

/* MEXT */
typedef struct MEXT {
	OBJECT_IDENTIFIER_t	*direct_reference	/* OPTIONAL */;
	long	*indirect_reference	/* OPTIONAL */;
	ObjectDescriptor_t	*data_value_descriptor	/* OPTIONAL */;
	struct mencoding {
		mencoding_PR present;
		union MEXT__mencoding_u {
			ANY_t	 single_ASN1_type;
			OCTET_STRING_t	 octet_aligned;
			BIT_STRING_t	 arbitrary;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} mencoding;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MEXT_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MEXT;

#ifdef __cplusplus
}
#endif

#endif	/* _MEXT_H_ */
