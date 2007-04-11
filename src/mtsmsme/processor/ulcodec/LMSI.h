#ifndef	_LMSI_H_
#define	_LMSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LMSI */
typedef OCTET_STRING_t	 LMSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LMSI;
asn_struct_free_f LMSI_free;
asn_struct_print_f LMSI_print;
asn_constr_check_f LMSI_constraint;
ber_type_decoder_f LMSI_decode_ber;
der_type_encoder_f LMSI_encode_der;
xer_type_decoder_f LMSI_decode_xer;
xer_type_encoder_f LMSI_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _LMSI_H_ */
