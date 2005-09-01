#ifndef _P_AbortCause_H_
#define _P_AbortCause_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>

typedef enum P_AbortCause {
  P_AbortCause_unrecognizedMessageType  = 0,
  P_AbortCause_unrecognizedTransactionID  = 1,
  P_AbortCause_badlyFormattedTransactionPortion = 2,
  P_AbortCause_incorrectTransactionPortion  = 3,
  P_AbortCause_resourceLimitation = 4
} P_AbortCause_e;


typedef int  P_AbortCause_t;

extern asn_TYPE_descriptor_t asn_DEF_P_AbortCause;
asn_struct_free_f P_AbortCause_free;
asn_struct_print_f P_AbortCause_print;
asn_constr_check_f P_AbortCause_constraint;
ber_type_decoder_f P_AbortCause_decode_ber;
der_type_encoder_f P_AbortCause_encode_der;
xer_type_decoder_f P_AbortCause_decode_xer;
xer_type_encoder_f P_AbortCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif  /* _P_AbortCause_H_ */
