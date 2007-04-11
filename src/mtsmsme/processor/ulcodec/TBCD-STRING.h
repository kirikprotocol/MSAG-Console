#ifndef	_TBCD_STRING_H_
#define	_TBCD_STRING_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TBCD-STRING */
typedef OCTET_STRING_t	 TBCD_STRING_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TBCD_STRING;
asn_struct_free_f TBCD_STRING_free;
asn_struct_print_f TBCD_STRING_print;
asn_constr_check_f TBCD_STRING_constraint;
ber_type_decoder_f TBCD_STRING_decode_ber;
der_type_encoder_f TBCD_STRING_encode_der;
xer_type_decoder_f TBCD_STRING_decode_xer;
xer_type_encoder_f TBCD_STRING_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TBCD_STRING_H_ */
