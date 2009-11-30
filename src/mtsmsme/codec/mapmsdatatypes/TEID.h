#ifndef	_TEID_H_
#define	_TEID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TEID */
typedef OCTET_STRING_t	 TEID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TEID;
asn_struct_free_f TEID_free;
asn_struct_print_f TEID_print;
asn_constr_check_f TEID_constraint;
ber_type_decoder_f TEID_decode_ber;
der_type_encoder_f TEID_encode_der;
xer_type_decoder_f TEID_decode_xer;
xer_type_encoder_f TEID_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TEID_H_ */
