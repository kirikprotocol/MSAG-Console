#ifndef	_TimerID_H_
#define	_TimerID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum TimerID {
	TimerID_tssf	= 0
} TimerID_e;

/* TimerID */
typedef long	 TimerID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TimerID;
asn_struct_free_f TimerID_free;
asn_struct_print_f TimerID_print;
asn_constr_check_f TimerID_constraint;
ber_type_decoder_f TimerID_decode_ber;
der_type_encoder_f TimerID_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TimerID_decode_xer;
xer_type_encoder_f TimerID_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TimerID_H_ */
