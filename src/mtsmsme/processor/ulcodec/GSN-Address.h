#ifndef	_GSN_Address_H_
#define	_GSN_Address_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GSN-Address */
typedef OCTET_STRING_t	 GSN_Address_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GSN_Address;
asn_struct_free_f GSN_Address_free;
asn_struct_print_f GSN_Address_print;
asn_constr_check_f GSN_Address_constraint;
ber_type_decoder_f GSN_Address_decode_ber;
der_type_encoder_f GSN_Address_encode_der;
xer_type_decoder_f GSN_Address_decode_xer;
xer_type_encoder_f GSN_Address_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _GSN_Address_H_ */
