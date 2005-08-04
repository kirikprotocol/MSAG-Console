#ifndef	_MO_SMSCause_H_
#define	_MO_SMSCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MO_SMSCause {
	MO_SMSCause_systemFailure	= 0,
	MO_SMSCause_unexpectedDataValue	= 1,
	MO_SMSCause_facilityNotSupported	= 2,
	MO_SMSCause_sM_DeliveryFailure	= 3,
	MO_SMSCause_releaseFromRadioInterface	= 4
} MO_SMSCause_e;

/* MO-SMSCause */
typedef long	 MO_SMSCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MO_SMSCause;
asn_struct_free_f MO_SMSCause_free;
asn_struct_print_f MO_SMSCause_print;
asn_constr_check_f MO_SMSCause_constraint;
ber_type_decoder_f MO_SMSCause_decode_ber;
der_type_encoder_f MO_SMSCause_encode_der;
xer_type_decoder_f MO_SMSCause_decode_xer;
xer_type_encoder_f MO_SMSCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _MO_SMSCause_H_ */
