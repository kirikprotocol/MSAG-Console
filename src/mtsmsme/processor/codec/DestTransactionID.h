#ifndef _DestTransactionID_H_
#define _DestTransactionID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <OCTET_STRING.h>



typedef OCTET_STRING_t   DestTransactionID_t;

extern asn_TYPE_descriptor_t asn_DEF_DestTransactionID;
asn_struct_free_f DestTransactionID_free;
asn_struct_print_f DestTransactionID_print;
asn_constr_check_f DestTransactionID_constraint;
ber_type_decoder_f DestTransactionID_decode_ber;
der_type_encoder_f DestTransactionID_encode_der;
xer_type_decoder_f DestTransactionID_decode_xer;
xer_type_encoder_f DestTransactionID_encode_xer;

#ifdef __cplusplus
}
#endif

#endif  /* _DestTransactionID_H_ */
