#ident "$Id$"

#ifndef	_Ext_NoRepCondTime_H_
#define	_Ext_NoRepCondTime_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ext-NoRepCondTime */
typedef long	 Ext_NoRepCondTime_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_NoRepCondTime;
asn_struct_free_f Ext_NoRepCondTime_free;
asn_struct_print_f Ext_NoRepCondTime_print;
asn_constr_check_f Ext_NoRepCondTime_constraint;
ber_type_decoder_f Ext_NoRepCondTime_decode_ber;
der_type_encoder_f Ext_NoRepCondTime_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Ext_NoRepCondTime_decode_xer;
xer_type_encoder_f Ext_NoRepCondTime_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_NoRepCondTime_H_ */
