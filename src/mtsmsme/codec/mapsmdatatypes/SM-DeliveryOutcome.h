#ifndef	_SM_DeliveryOutcome_H_
#define	_SM_DeliveryOutcome_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SM_DeliveryOutcome {
	SM_DeliveryOutcome_memoryCapacityExceeded	= 0,
	SM_DeliveryOutcome_absentSubscriber	= 1,
	SM_DeliveryOutcome_successfulTransfer	= 2
} e_SM_DeliveryOutcome;

/* SM-DeliveryOutcome */
typedef long	 SM_DeliveryOutcome_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_DeliveryOutcome;
asn_struct_free_f SM_DeliveryOutcome_free;
asn_struct_print_f SM_DeliveryOutcome_print;
asn_constr_check_f SM_DeliveryOutcome_constraint;
ber_type_decoder_f SM_DeliveryOutcome_decode_ber;
der_type_encoder_f SM_DeliveryOutcome_encode_der;
xer_type_decoder_f SM_DeliveryOutcome_decode_xer;
xer_type_encoder_f SM_DeliveryOutcome_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_DeliveryOutcome_H_ */
