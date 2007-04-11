#ifndef	_AbsentSubscriberReason_H_
#define	_AbsentSubscriberReason_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AbsentSubscriberReason {
	AbsentSubscriberReason_imsiDetach	= 0,
	AbsentSubscriberReason_restrictedArea	= 1,
	AbsentSubscriberReason_noPageResponse	= 2,
	/*
	 * Enumeration is extensible
	 */
	AbsentSubscriberReason_purgedMS	= 3
} e_AbsentSubscriberReason;

/* AbsentSubscriberReason */
typedef long	 AbsentSubscriberReason_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AbsentSubscriberReason;
asn_struct_free_f AbsentSubscriberReason_free;
asn_struct_print_f AbsentSubscriberReason_print;
asn_constr_check_f AbsentSubscriberReason_constraint;
ber_type_decoder_f AbsentSubscriberReason_decode_ber;
der_type_encoder_f AbsentSubscriberReason_encode_der;
xer_type_decoder_f AbsentSubscriberReason_decode_xer;
xer_type_encoder_f AbsentSubscriberReason_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _AbsentSubscriberReason_H_ */
