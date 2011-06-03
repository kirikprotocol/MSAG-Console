#ifndef	_DefaultCallHandling_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DefaultCallHandling_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DefaultCallHandling {
	DefaultCallHandling_continueCall	= 0,
	DefaultCallHandling_releaseCall	= 1
	/*
	 * Enumeration is extensible
	 */
} DefaultCallHandling_e;

/* DefaultCallHandling */
typedef long	 DefaultCallHandling_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DefaultCallHandling;
asn_struct_free_f DefaultCallHandling_free;
asn_struct_print_f DefaultCallHandling_print;
asn_constr_check_f DefaultCallHandling_constraint;
ber_type_decoder_f DefaultCallHandling_decode_ber;
der_type_encoder_f DefaultCallHandling_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f DefaultCallHandling_decode_xer;
xer_type_encoder_f DefaultCallHandling_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _DefaultCallHandling_H_ */
