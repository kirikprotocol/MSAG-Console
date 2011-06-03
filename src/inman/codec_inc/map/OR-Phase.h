#ifndef	_OR_Phase_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_OR_Phase_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OR-Phase */
typedef long	 OR_Phase_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OR_Phase;
asn_struct_free_f OR_Phase_free;
asn_struct_print_f OR_Phase_print;
asn_constr_check_f OR_Phase_constraint;
ber_type_decoder_f OR_Phase_decode_ber;
der_type_encoder_f OR_Phase_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f OR_Phase_decode_xer;
xer_type_encoder_f OR_Phase_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _OR_Phase_H_ */
