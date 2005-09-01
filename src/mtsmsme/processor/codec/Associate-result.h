#ifndef _Associate_result_H_
#define _Associate_result_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>

typedef enum Associate_result {
  Associate_result_accepted = 0,
  Associate_result_reject_permanent = 1
} Associate_result_e;


typedef int  Associate_result_t;

extern asn_TYPE_descriptor_t asn_DEF_Associate_result;
asn_struct_free_f Associate_result_free;
asn_struct_print_f Associate_result_print;
asn_constr_check_f Associate_result_constraint;
ber_type_decoder_f Associate_result_decode_ber;
der_type_encoder_f Associate_result_encode_der;
xer_type_decoder_f Associate_result_decode_xer;
xer_type_encoder_f Associate_result_encode_xer;

#ifdef __cplusplus
}
#endif

#endif  /* _Associate_result_H_ */
