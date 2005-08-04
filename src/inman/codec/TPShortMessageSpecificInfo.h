#ifndef	_TPShortMessageSpecificInfo_H_
#define	_TPShortMessageSpecificInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPShortMessageSpecificInfo */
typedef OCTET_STRING_t	 TPShortMessageSpecificInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TPShortMessageSpecificInfo;
asn_struct_free_f TPShortMessageSpecificInfo_free;
asn_struct_print_f TPShortMessageSpecificInfo_print;
asn_constr_check_f TPShortMessageSpecificInfo_constraint;
ber_type_decoder_f TPShortMessageSpecificInfo_decode_ber;
der_type_encoder_f TPShortMessageSpecificInfo_encode_der;
xer_type_decoder_f TPShortMessageSpecificInfo_decode_xer;
xer_type_encoder_f TPShortMessageSpecificInfo_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TPShortMessageSpecificInfo_H_ */
