#ifndef	_AllowedServices_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_AllowedServices_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AllowedServices {
	AllowedServices_firstServiceAllowed	= 0,
	AllowedServices_secondServiceAllowed	= 1
} AllowedServices_e;

/* AllowedServices */
typedef BIT_STRING_t	 AllowedServices_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AllowedServices;
asn_struct_free_f AllowedServices_free;
asn_struct_print_f AllowedServices_print;
asn_constr_check_f AllowedServices_constraint;
ber_type_decoder_f AllowedServices_decode_ber;
der_type_encoder_f AllowedServices_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AllowedServices_decode_xer;
xer_type_encoder_f AllowedServices_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AllowedServices_H_ */
