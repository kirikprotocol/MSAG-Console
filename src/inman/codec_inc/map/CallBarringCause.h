#ifndef	_CallBarringCause_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CallBarringCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CallBarringCause {
	CallBarringCause_barringServiceActive	= 0,
	CallBarringCause_operatorBarring	= 1
} CallBarringCause_e;

/* CallBarringCause */
typedef long	 CallBarringCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallBarringCause;
asn_struct_free_f CallBarringCause_free;
asn_struct_print_f CallBarringCause_print;
asn_constr_check_f CallBarringCause_constraint;
ber_type_decoder_f CallBarringCause_decode_ber;
der_type_encoder_f CallBarringCause_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f CallBarringCause_decode_xer;
xer_type_encoder_f CallBarringCause_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _CallBarringCause_H_ */
