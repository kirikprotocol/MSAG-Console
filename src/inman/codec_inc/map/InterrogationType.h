#ifndef	_InterrogationType_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_InterrogationType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum InterrogationType {
	InterrogationType_basicCall	= 0,
	InterrogationType_forwarding	= 1
} InterrogationType_e;

/* InterrogationType */
typedef long	 InterrogationType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InterrogationType;
asn_struct_free_f InterrogationType_free;
asn_struct_print_f InterrogationType_print;
asn_constr_check_f InterrogationType_constraint;
ber_type_decoder_f InterrogationType_decode_ber;
der_type_encoder_f InterrogationType_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f InterrogationType_decode_xer;
xer_type_encoder_f InterrogationType_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _InterrogationType_H_ */
