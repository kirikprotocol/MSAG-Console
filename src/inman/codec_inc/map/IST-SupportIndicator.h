#ifndef	_IST_SupportIndicator_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_IST_SupportIndicator_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum IST_SupportIndicator {
	IST_SupportIndicator_basicISTSupported	= 0,
	IST_SupportIndicator_istCommandSupported	= 1
	/*
	 * Enumeration is extensible
	 */
} IST_SupportIndicator_e;

/* IST-SupportIndicator */
typedef long	 IST_SupportIndicator_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IST_SupportIndicator;
asn_struct_free_f IST_SupportIndicator_free;
asn_struct_print_f IST_SupportIndicator_print;
asn_constr_check_f IST_SupportIndicator_constraint;
ber_type_decoder_f IST_SupportIndicator_decode_ber;
der_type_encoder_f IST_SupportIndicator_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f IST_SupportIndicator_decode_xer;
xer_type_encoder_f IST_SupportIndicator_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _IST_SupportIndicator_H_ */
