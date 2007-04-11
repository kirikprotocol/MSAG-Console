#ifndef	_RoamingNotAllowedCause_H_
#define	_RoamingNotAllowedCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RoamingNotAllowedCause {
	RoamingNotAllowedCause_plmnRoamingNotAllowed	= 0,
	RoamingNotAllowedCause_operatorDeterminedBarring	= 3
} e_RoamingNotAllowedCause;

/* RoamingNotAllowedCause */
typedef long	 RoamingNotAllowedCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RoamingNotAllowedCause;
asn_struct_free_f RoamingNotAllowedCause_free;
asn_struct_print_f RoamingNotAllowedCause_print;
asn_constr_check_f RoamingNotAllowedCause_constraint;
ber_type_decoder_f RoamingNotAllowedCause_decode_ber;
der_type_encoder_f RoamingNotAllowedCause_encode_der;
xer_type_decoder_f RoamingNotAllowedCause_decode_xer;
xer_type_encoder_f RoamingNotAllowedCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _RoamingNotAllowedCause_H_ */
