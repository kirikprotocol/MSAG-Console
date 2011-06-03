#ifndef	_ContextId_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ContextId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ContextId */
typedef long	 ContextId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ContextId;
asn_struct_free_f ContextId_free;
asn_struct_print_f ContextId_print;
asn_constr_check_f ContextId_constraint;
ber_type_decoder_f ContextId_decode_ber;
der_type_encoder_f ContextId_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ContextId_decode_xer;
xer_type_encoder_f ContextId_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ContextId_H_ */
