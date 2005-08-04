#ifndef	_Priority_H_
#define	_Priority_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Priority */
typedef long	 Priority_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Priority;
asn_struct_free_f Priority_free;
asn_struct_print_f Priority_print;
asn_constr_check_f Priority_constraint;
ber_type_decoder_f Priority_decode_ber;
der_type_encoder_f Priority_encode_der;
xer_type_decoder_f Priority_decode_xer;
xer_type_encoder_f Priority_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _Priority_H_ */
