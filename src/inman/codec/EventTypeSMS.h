#ifndef	_EventTypeSMS_H_
#define	_EventTypeSMS_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum EventTypeSMS {
	EventTypeSMS_sms_CollectedInfo	= 1,
	EventTypeSMS_o_smsFailure	= 2,
	EventTypeSMS_o_smsSubmission	= 3,
	EventTypeSMS_sms_DeliveryRequested	= 11,
	EventTypeSMS_t_smsFailure	= 12,
	EventTypeSMS_t_smsDelivery	= 13
} EventTypeSMS_e;

/* EventTypeSMS */
typedef long	 EventTypeSMS_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EventTypeSMS;
asn_struct_free_f EventTypeSMS_free;
asn_struct_print_f EventTypeSMS_print;
asn_constr_check_f EventTypeSMS_constraint;
ber_type_decoder_f EventTypeSMS_decode_ber;
der_type_encoder_f EventTypeSMS_encode_der;
xer_type_decoder_f EventTypeSMS_decode_xer;
xer_type_encoder_f EventTypeSMS_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _EventTypeSMS_H_ */
