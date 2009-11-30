#ifndef	_LocationNumber_H_
#define	_LocationNumber_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LocationNumber */
typedef OCTET_STRING_t	 LocationNumber_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationNumber;
asn_struct_free_f LocationNumber_free;
asn_struct_print_f LocationNumber_print;
asn_constr_check_f LocationNumber_constraint;
ber_type_decoder_f LocationNumber_decode_ber;
der_type_encoder_f LocationNumber_encode_der;
xer_type_decoder_f LocationNumber_decode_xer;
xer_type_encoder_f LocationNumber_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _LocationNumber_H_ */
