#ifndef	_Ext_ProtocolId_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext_ProtocolId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Ext_ProtocolId {
	Ext_ProtocolId_ets_300356	= 1
	/*
	 * Enumeration is extensible
	 */
} Ext_ProtocolId_e;

/* Ext-ProtocolId */
typedef long	 Ext_ProtocolId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext_ProtocolId;
asn_struct_free_f Ext_ProtocolId_free;
asn_struct_print_f Ext_ProtocolId_print;
asn_constr_check_f Ext_ProtocolId_constraint;
ber_type_decoder_f Ext_ProtocolId_decode_ber;
der_type_encoder_f Ext_ProtocolId_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Ext_ProtocolId_decode_xer;
xer_type_encoder_f Ext_ProtocolId_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Ext_ProtocolId_H_ */
