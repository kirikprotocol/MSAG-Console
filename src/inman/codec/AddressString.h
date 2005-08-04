#ifndef	_AddressString_H_
#define	_AddressString_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AddressString */
typedef OCTET_STRING_t	 AddressString_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AddressString;
asn_struct_free_f AddressString_free;
asn_struct_print_f AddressString_print;
asn_constr_check_f AddressString_constraint;
ber_type_decoder_f AddressString_decode_ber;
der_type_encoder_f AddressString_encode_der;
xer_type_decoder_f AddressString_decode_xer;
xer_type_encoder_f AddressString_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _AddressString_H_ */
