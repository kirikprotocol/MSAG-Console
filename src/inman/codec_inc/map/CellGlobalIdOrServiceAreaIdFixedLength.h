#ifndef	_CellGlobalIdOrServiceAreaIdFixedLength_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CellGlobalIdOrServiceAreaIdFixedLength_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CellGlobalIdOrServiceAreaIdFixedLength */
typedef OCTET_STRING_t	 CellGlobalIdOrServiceAreaIdFixedLength_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CellGlobalIdOrServiceAreaIdFixedLength;
asn_struct_free_f CellGlobalIdOrServiceAreaIdFixedLength_free;
asn_struct_print_f CellGlobalIdOrServiceAreaIdFixedLength_print;
asn_constr_check_f CellGlobalIdOrServiceAreaIdFixedLength_constraint;
ber_type_decoder_f CellGlobalIdOrServiceAreaIdFixedLength_decode_ber;
der_type_encoder_f CellGlobalIdOrServiceAreaIdFixedLength_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f CellGlobalIdOrServiceAreaIdFixedLength_decode_xer;
xer_type_encoder_f CellGlobalIdOrServiceAreaIdFixedLength_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _CellGlobalIdOrServiceAreaIdFixedLength_H_ */
