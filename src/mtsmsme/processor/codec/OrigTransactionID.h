#ifndef _OrigTransactionID_H_
#define _OrigTransactionID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <OCTET_STRING.h>



typedef OCTET_STRING_t   OrigTransactionID_t;

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

#endif  /* _OrigTransactionID_H_ */
