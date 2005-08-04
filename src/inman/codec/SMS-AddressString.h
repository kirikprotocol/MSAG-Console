#ifndef	_SMS_AddressString_H_
#define	_SMS_AddressString_H_


#include <asn_application.h>

/* Including external dependencies */
#include <AddressString.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SMS-AddressString */
typedef AddressString_t	 SMS_AddressString_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMS_AddressString;
asn_struct_free_f SMS_AddressString_free;
asn_struct_print_f SMS_AddressString_print;
asn_constr_check_f SMS_AddressString_constraint;
ber_type_decoder_f SMS_AddressString_decode_ber;
der_type_encoder_f SMS_AddressString_encode_der;
xer_type_decoder_f SMS_AddressString_decode_xer;
xer_type_encoder_f SMS_AddressString_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SMS_AddressString_H_ */
