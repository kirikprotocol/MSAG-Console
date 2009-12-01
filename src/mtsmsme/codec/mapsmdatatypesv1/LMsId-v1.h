#ifndef	_LMsId_v1_H_
#define	_LMsId_v1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LMsId-v1 */
typedef OCTET_STRING_t	 LMsId_v1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LMsId_v1;
asn_struct_free_f LMsId_v1_free;
asn_struct_print_f LMsId_v1_print;
asn_constr_check_f LMsId_v1_constraint;
ber_type_decoder_f LMsId_v1_decode_ber;
der_type_encoder_f LMsId_v1_encode_der;
xer_type_decoder_f LMsId_v1_decode_xer;
xer_type_encoder_f LMsId_v1_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _LMsId_v1_H_ */
