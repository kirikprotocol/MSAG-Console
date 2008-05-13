#ifndef	_SM_RP_SMEA_H_
#define	_SM_RP_SMEA_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SM-RP-SMEA */
typedef OCTET_STRING_t	 SM_RP_SMEA_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_RP_SMEA;
asn_struct_free_f SM_RP_SMEA_free;
asn_struct_print_f SM_RP_SMEA_print;
asn_constr_check_f SM_RP_SMEA_constraint;
ber_type_decoder_f SM_RP_SMEA_decode_ber;
der_type_encoder_f SM_RP_SMEA_encode_der;
xer_type_decoder_f SM_RP_SMEA_decode_xer;
xer_type_encoder_f SM_RP_SMEA_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_RP_SMEA_H_ */
