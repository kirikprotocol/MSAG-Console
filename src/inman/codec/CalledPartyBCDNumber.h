#ifndef	_CalledPartyBCDNumber_H_
#define	_CalledPartyBCDNumber_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CalledPartyBCDNumber */
typedef OCTET_STRING_t	 CalledPartyBCDNumber_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CalledPartyBCDNumber;
asn_struct_free_f CalledPartyBCDNumber_free;
asn_struct_print_f CalledPartyBCDNumber_print;
asn_constr_check_f CalledPartyBCDNumber_constraint;
ber_type_decoder_f CalledPartyBCDNumber_decode_ber;
der_type_encoder_f CalledPartyBCDNumber_encode_der;
xer_type_decoder_f CalledPartyBCDNumber_decode_xer;
xer_type_encoder_f CalledPartyBCDNumber_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CalledPartyBCDNumber_H_ */
