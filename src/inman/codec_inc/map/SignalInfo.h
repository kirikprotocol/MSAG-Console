#ifndef	_SignalInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SignalInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SignalInfo */
typedef OCTET_STRING_t	 SignalInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SignalInfo;
asn_struct_free_f SignalInfo_free;
asn_struct_print_f SignalInfo_print;
asn_constr_check_f SignalInfo_constraint;
ber_type_decoder_f SignalInfo_decode_ber;
der_type_encoder_f SignalInfo_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f SignalInfo_decode_xer;
xer_type_encoder_f SignalInfo_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _SignalInfo_H_ */
