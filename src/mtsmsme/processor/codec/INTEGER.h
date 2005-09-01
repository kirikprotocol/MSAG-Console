#ifndef _INTEGER_H_
#define _INTEGER_H_

#include <asn_application.h>
#include <asn_codecs_prim.h>

typedef ASN__PRIMITIVE_TYPE_t INTEGER_t;

extern asn_TYPE_descriptor_t asn_DEF_INTEGER;

asn_struct_print_f INTEGER_print;
ber_type_decoder_f INTEGER_decode_ber;
der_type_encoder_f INTEGER_encode_der;
xer_type_decoder_f INTEGER_decode_xer;
xer_type_encoder_f INTEGER_encode_xer;

/***********************************
 * Some handy conversion routines. *
 ***********************************/

/*
 * Returns 0 if it was possible to convert, -1 otherwise.
 * -1/EINVAL: Mandatory argument missing
 * -1/ERANGE: Value encoded is out of range for long representation
 * -1/ENOMEM: Memory allocation failed (in asn_long2INTEGER()).
 */
int asn_INTEGER2long(const INTEGER_t *i, long *l);
int asn_long2INTEGER(INTEGER_t *i, long l);

#endif  /* _INTEGER_H_ */
