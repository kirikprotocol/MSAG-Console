#ifndef	_OrigTransactionID_H_
#define	_OrigTransactionID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OrigTransactionID */
typedef OCTET_STRING_t	 OrigTransactionID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OrigTransactionID;
asn_struct_free_f OrigTransactionID_free;
asn_struct_print_f OrigTransactionID_print;
asn_constr_check_f OrigTransactionID_constraint;
ber_type_decoder_f OrigTransactionID_decode_ber;
der_type_encoder_f OrigTransactionID_encode_der;
xer_type_decoder_f OrigTransactionID_decode_xer;
xer_type_encoder_f OrigTransactionID_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _OrigTransactionID_H_ */
