#ifndef	_AlertingPattern_H_
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define	_AlertingPattern_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AlertingPattern */
typedef OCTET_STRING_t	 AlertingPattern_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AlertingPattern;
asn_struct_free_f AlertingPattern_free;
asn_struct_print_f AlertingPattern_print;
asn_constr_check_f AlertingPattern_constraint;
ber_type_decoder_f AlertingPattern_decode_ber;
der_type_encoder_f AlertingPattern_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AlertingPattern_decode_xer;
xer_type_encoder_f AlertingPattern_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AlertingPattern_H_ */
