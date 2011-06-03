#ifndef	_OfferedCamel4CSIs_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_OfferedCamel4CSIs_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum OfferedCamel4CSIs {
	OfferedCamel4CSIs_o_csi	= 0,
	OfferedCamel4CSIs_d_csi	= 1,
	OfferedCamel4CSIs_vt_csi	= 2,
	OfferedCamel4CSIs_t_csi	= 3,
	OfferedCamel4CSIs_mt_sms_csi	= 4,
	OfferedCamel4CSIs_mg_csi	= 5,
	OfferedCamel4CSIs_psi_enhancements	= 6
} OfferedCamel4CSIs_e;

/* OfferedCamel4CSIs */
typedef BIT_STRING_t	 OfferedCamel4CSIs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OfferedCamel4CSIs;
asn_struct_free_f OfferedCamel4CSIs_free;
asn_struct_print_f OfferedCamel4CSIs_print;
asn_constr_check_f OfferedCamel4CSIs_constraint;
ber_type_decoder_f OfferedCamel4CSIs_decode_ber;
der_type_encoder_f OfferedCamel4CSIs_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f OfferedCamel4CSIs_decode_xer;
xer_type_encoder_f OfferedCamel4CSIs_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _OfferedCamel4CSIs_H_ */
