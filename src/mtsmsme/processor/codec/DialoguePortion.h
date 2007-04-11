#ifndef	_DialoguePortion_H_
#define	_DialoguePortion_H_


#include <asn_application.h>

/* Including external dependencies */
#include "EXT.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DialoguePortion */
typedef EXT_t	 DialoguePortion_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DialoguePortion;
asn_struct_free_f DialoguePortion_free;
asn_struct_print_f DialoguePortion_print;
asn_constr_check_f DialoguePortion_constraint;
ber_type_decoder_f DialoguePortion_decode_ber;
der_type_encoder_f DialoguePortion_encode_der;
xer_type_decoder_f DialoguePortion_decode_xer;
xer_type_encoder_f DialoguePortion_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _DialoguePortion_H_ */
