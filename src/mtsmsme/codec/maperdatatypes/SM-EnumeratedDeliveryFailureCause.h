#ifndef	_SM_EnumeratedDeliveryFailureCause_H_
#define	_SM_EnumeratedDeliveryFailureCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SM_EnumeratedDeliveryFailureCause {
	SM_EnumeratedDeliveryFailureCause_memoryCapacityExceeded	= 0,
	SM_EnumeratedDeliveryFailureCause_equipmentProtocolError	= 1,
	SM_EnumeratedDeliveryFailureCause_equipmentNotSM_Equipped	= 2,
	SM_EnumeratedDeliveryFailureCause_unknownServiceCentre	= 3,
	SM_EnumeratedDeliveryFailureCause_sc_Congestion	= 4,
	SM_EnumeratedDeliveryFailureCause_invalidSME_Address	= 5,
	SM_EnumeratedDeliveryFailureCause_subscriberNotSC_Subscriber	= 6
} e_SM_EnumeratedDeliveryFailureCause;

/* SM-EnumeratedDeliveryFailureCause */
typedef long	 SM_EnumeratedDeliveryFailureCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SM_EnumeratedDeliveryFailureCause;
asn_struct_free_f SM_EnumeratedDeliveryFailureCause_free;
asn_struct_print_f SM_EnumeratedDeliveryFailureCause_print;
asn_constr_check_f SM_EnumeratedDeliveryFailureCause_constraint;
ber_type_decoder_f SM_EnumeratedDeliveryFailureCause_decode_ber;
der_type_encoder_f SM_EnumeratedDeliveryFailureCause_encode_der;
xer_type_decoder_f SM_EnumeratedDeliveryFailureCause_decode_xer;
xer_type_encoder_f SM_EnumeratedDeliveryFailureCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SM_EnumeratedDeliveryFailureCause_H_ */
