#ifndef	_Ext_TeleserviceCode_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext_TeleserviceCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ext-TeleserviceCode */
typedef OCTET_STRING_t	 Ext_TeleserviceCode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_TeleserviceCode;
asn_struct_free_f Ext_TeleserviceCode_free;
asn_struct_print_f Ext_TeleserviceCode_print;
asn_constr_check_f Ext_TeleserviceCode_constraint;
ber_type_decoder_f Ext_TeleserviceCode_decode_ber;
der_type_encoder_f Ext_TeleserviceCode_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Ext_TeleserviceCode_decode_xer;
xer_type_encoder_f Ext_TeleserviceCode_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_TeleserviceCode_H_ */
