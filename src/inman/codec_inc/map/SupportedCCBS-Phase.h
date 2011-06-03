#ifndef	_SupportedCCBS_Phase_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SupportedCCBS_Phase_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SupportedCCBS-Phase */
typedef long	 SupportedCCBS_Phase_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SupportedCCBS_Phase;
asn_struct_free_f SupportedCCBS_Phase_free;
asn_struct_print_f SupportedCCBS_Phase_print;
asn_constr_check_f SupportedCCBS_Phase_constraint;
ber_type_decoder_f SupportedCCBS_Phase_decode_ber;
der_type_encoder_f SupportedCCBS_Phase_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f SupportedCCBS_Phase_decode_xer;
xer_type_encoder_f SupportedCCBS_Phase_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _SupportedCCBS_Phase_H_ */
