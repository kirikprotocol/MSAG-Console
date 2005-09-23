#ifndef	_TimerValue_H_
#define	_TimerValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Integer4.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TimerValue */
typedef Integer4_t	 TimerValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TimerValue;
asn_struct_free_f TimerValue_free;
asn_struct_print_f TimerValue_print;
asn_constr_check_f TimerValue_constraint;
ber_type_decoder_f TimerValue_decode_ber;
der_type_encoder_f TimerValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TimerValue_decode_xer;
xer_type_encoder_f TimerValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TimerValue_H_ */
