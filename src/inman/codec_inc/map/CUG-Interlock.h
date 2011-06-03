#ifndef	_CUG_Interlock_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CUG_Interlock_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CUG-Interlock */
typedef OCTET_STRING_t	 CUG_Interlock_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CUG_Interlock;
asn_struct_free_f CUG_Interlock_free;
asn_struct_print_f CUG_Interlock_print;
asn_constr_check_f CUG_Interlock_constraint;
ber_type_decoder_f CUG_Interlock_decode_ber;
der_type_encoder_f CUG_Interlock_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f CUG_Interlock_decode_xer;
xer_type_encoder_f CUG_Interlock_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _CUG_Interlock_H_ */
