#ifndef	_IST_AlertTimerValue_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_IST_AlertTimerValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IST-AlertTimerValue */
typedef long	 IST_AlertTimerValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IST_AlertTimerValue;
asn_struct_free_f IST_AlertTimerValue_free;
asn_struct_print_f IST_AlertTimerValue_print;
asn_constr_check_f IST_AlertTimerValue_constraint;
ber_type_decoder_f IST_AlertTimerValue_decode_ber;
der_type_encoder_f IST_AlertTimerValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f IST_AlertTimerValue_decode_xer;
xer_type_encoder_f IST_AlertTimerValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _IST_AlertTimerValue_H_ */
