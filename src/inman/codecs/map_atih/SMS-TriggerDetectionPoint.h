#ident "$Id$"

#ifndef	_SMS_TriggerDetectionPoint_H_
#define	_SMS_TriggerDetectionPoint_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SMS_TriggerDetectionPoint {
	SMS_TriggerDetectionPoint_sms_CollectedInfo	= 1,
	/*
	 * Enumeration is extensible
	 */
	SMS_TriggerDetectionPoint_sms_DeliveryRequest	= 2
} SMS_TriggerDetectionPoint_e;

/* SMS-TriggerDetectionPoint */
typedef long	 SMS_TriggerDetectionPoint_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMS_TriggerDetectionPoint;
asn_struct_free_f SMS_TriggerDetectionPoint_free;
asn_struct_print_f SMS_TriggerDetectionPoint_print;
asn_constr_check_f SMS_TriggerDetectionPoint_constraint;
ber_type_decoder_f SMS_TriggerDetectionPoint_decode_ber;
der_type_encoder_f SMS_TriggerDetectionPoint_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f SMS_TriggerDetectionPoint_decode_xer;
xer_type_encoder_f SMS_TriggerDetectionPoint_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _SMS_TriggerDetectionPoint_H_ */
