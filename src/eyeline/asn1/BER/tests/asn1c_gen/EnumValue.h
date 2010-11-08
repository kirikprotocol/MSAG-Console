#ident "$Id$"

#ifndef	_EnumValue_H_
#define	_EnumValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum EnumValue {
	EnumValue_red	= 0,
	EnumValue_green	= 1,
	EnumValue_blue	= 4,
	EnumValue_alpha	= 5
	/*
	 * Enumeration is extensible
	 */
} EnumValue_e;

/* EnumValue */
typedef long	 EnumValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EnumValue;
asn_struct_free_f EnumValue_free;
asn_struct_print_f EnumValue_print;
asn_constr_check_f EnumValue_constraint;
ber_type_decoder_f EnumValue_decode_ber;
der_type_encoder_f EnumValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f EnumValue_decode_xer;
xer_type_encoder_f EnumValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _EnumValue_H_ */
