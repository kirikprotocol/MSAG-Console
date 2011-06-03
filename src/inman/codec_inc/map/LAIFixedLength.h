#ifndef	_LAIFixedLength_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_LAIFixedLength_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LAIFixedLength */
typedef OCTET_STRING_t	 LAIFixedLength_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LAIFixedLength;
asn_struct_free_f LAIFixedLength_free;
asn_struct_print_f LAIFixedLength_print;
asn_constr_check_f LAIFixedLength_constraint;
ber_type_decoder_f LAIFixedLength_decode_ber;
der_type_encoder_f LAIFixedLength_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f LAIFixedLength_decode_xer;
xer_type_encoder_f LAIFixedLength_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _LAIFixedLength_H_ */
