#ifndef _GeneralProblem_H_
#define _GeneralProblem_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>

typedef enum GeneralProblem {
  GeneralProblem_unrecognizedComponent  = 0,
  GeneralProblem_mistypedComponent  = 1,
  GeneralProblem_badlyStructuredComponent = 2
} GeneralProblem_e;


typedef int  GeneralProblem_t;

extern asn_TYPE_descriptor_t asn_DEF_GeneralProblem;
asn_struct_free_f GeneralProblem_free;
asn_struct_print_f GeneralProblem_print;
asn_constr_check_f GeneralProblem_constraint;
ber_type_decoder_f GeneralProblem_decode_ber;
der_type_encoder_f GeneralProblem_encode_der;
xer_type_decoder_f GeneralProblem_decode_xer;
xer_type_encoder_f GeneralProblem_encode_xer;

#ifdef __cplusplus
}
#endif

#endif  /* _GeneralProblem_H_ */
