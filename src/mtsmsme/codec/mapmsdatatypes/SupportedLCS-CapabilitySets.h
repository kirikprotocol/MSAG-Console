#ifndef	_SupportedLCS_CapabilitySets_H_
#define	_SupportedLCS_CapabilitySets_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SupportedLCS_CapabilitySets {
	SupportedLCS_CapabilitySets_lcsCapabilitySet1	= 0,
	SupportedLCS_CapabilitySets_lcsCapabilitySet2	= 1,
	SupportedLCS_CapabilitySets_lcsCapabilitySet3	= 2,
	SupportedLCS_CapabilitySets_lcsCapabilitySet4	= 3
} e_SupportedLCS_CapabilitySets;

/* SupportedLCS-CapabilitySets */
typedef BIT_STRING_t	 SupportedLCS_CapabilitySets_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SupportedLCS_CapabilitySets;
asn_struct_free_f SupportedLCS_CapabilitySets_free;
asn_struct_print_f SupportedLCS_CapabilitySets_print;
asn_constr_check_f SupportedLCS_CapabilitySets_constraint;
ber_type_decoder_f SupportedLCS_CapabilitySets_decode_ber;
der_type_encoder_f SupportedLCS_CapabilitySets_encode_der;
xer_type_decoder_f SupportedLCS_CapabilitySets_decode_xer;
xer_type_encoder_f SupportedLCS_CapabilitySets_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SupportedLCS_CapabilitySets_H_ */
