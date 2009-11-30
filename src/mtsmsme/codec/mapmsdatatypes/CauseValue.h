#ifndef	_CauseValue_H_
#define	_CauseValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CauseValue */
typedef OCTET_STRING_t	 CauseValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CauseValue;
asn_struct_free_f CauseValue_free;
asn_struct_print_f CauseValue_print;
asn_constr_check_f CauseValue_constraint;
ber_type_decoder_f CauseValue_decode_ber;
der_type_encoder_f CauseValue_encode_der;
xer_type_decoder_f CauseValue_decode_xer;
xer_type_encoder_f CauseValue_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CauseValue_H_ */
