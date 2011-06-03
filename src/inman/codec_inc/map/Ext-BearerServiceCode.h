#ifndef	_Ext_BearerServiceCode_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext_BearerServiceCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ext-BearerServiceCode */
typedef OCTET_STRING_t	 Ext_BearerServiceCode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_BearerServiceCode;
asn_struct_free_f Ext_BearerServiceCode_free;
asn_struct_print_f Ext_BearerServiceCode_print;
asn_constr_check_f Ext_BearerServiceCode_constraint;
ber_type_decoder_f Ext_BearerServiceCode_decode_ber;
der_type_encoder_f Ext_BearerServiceCode_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Ext_BearerServiceCode_decode_xer;
xer_type_encoder_f Ext_BearerServiceCode_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_BearerServiceCode_H_ */
