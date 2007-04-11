#ifndef	_SupportedCamelPhases_H_
#define	_SupportedCamelPhases_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SupportedCamelPhases {
	SupportedCamelPhases_phase1	= 0,
	SupportedCamelPhases_phase2	= 1,
	SupportedCamelPhases_phase3	= 2,
	SupportedCamelPhases_phase4	= 3
} e_SupportedCamelPhases;

/* SupportedCamelPhases */
typedef BIT_STRING_t	 SupportedCamelPhases_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SupportedCamelPhases;
asn_struct_free_f SupportedCamelPhases_free;
asn_struct_print_f SupportedCamelPhases_print;
asn_constr_check_f SupportedCamelPhases_constraint;
ber_type_decoder_f SupportedCamelPhases_decode_ber;
der_type_encoder_f SupportedCamelPhases_encode_der;
xer_type_decoder_f SupportedCamelPhases_decode_xer;
xer_type_encoder_f SupportedCamelPhases_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SupportedCamelPhases_H_ */
