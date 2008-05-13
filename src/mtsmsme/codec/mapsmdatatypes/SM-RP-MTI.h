#ifndef	_SM_RP_MTI_H_
#define	_SM_RP_MTI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SM-RP-MTI */
typedef long	 SM_RP_MTI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_RP_MTI;
asn_struct_free_f SM_RP_MTI_free;
asn_struct_print_f SM_RP_MTI_print;
asn_constr_check_f SM_RP_MTI_constraint;
ber_type_decoder_f SM_RP_MTI_decode_ber;
der_type_encoder_f SM_RP_MTI_encode_der;
xer_type_decoder_f SM_RP_MTI_decode_xer;
xer_type_encoder_f SM_RP_MTI_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_RP_MTI_H_ */
