#ifndef	_NumberPortabilityStatus_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_NumberPortabilityStatus_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NumberPortabilityStatus {
	NumberPortabilityStatus_notKnownToBePorted	= 0,
	NumberPortabilityStatus_ownNumberPortedOut	= 1,
	NumberPortabilityStatus_foreignNumberPortedToForeignNetwork	= 2,
	/*
	 * Enumeration is extensible
	 */
	NumberPortabilityStatus_ownNumberNotPortedOut	= 4,
	NumberPortabilityStatus_foreignNumberPortedIn	= 5
} NumberPortabilityStatus_e;

/* NumberPortabilityStatus */
typedef long	 NumberPortabilityStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NumberPortabilityStatus;
asn_struct_free_f NumberPortabilityStatus_free;
asn_struct_print_f NumberPortabilityStatus_print;
asn_constr_check_f NumberPortabilityStatus_constraint;
ber_type_decoder_f NumberPortabilityStatus_decode_ber;
der_type_encoder_f NumberPortabilityStatus_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f NumberPortabilityStatus_decode_xer;
xer_type_encoder_f NumberPortabilityStatus_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _NumberPortabilityStatus_H_ */
