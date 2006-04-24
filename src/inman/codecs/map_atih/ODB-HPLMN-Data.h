#ident "$Id$"

#ifndef	_ODB_HPLMN_Data_H_
#define	_ODB_HPLMN_Data_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ODB_HPLMN_Data {
	ODB_HPLMN_Data_plmn_SpecificBarringType1	= 0,
	ODB_HPLMN_Data_plmn_SpecificBarringType2	= 1,
	ODB_HPLMN_Data_plmn_SpecificBarringType3	= 2,
	ODB_HPLMN_Data_plmn_SpecificBarringType4	= 3
} ODB_HPLMN_Data_e;

/* ODB-HPLMN-Data */
typedef BIT_STRING_t	 ODB_HPLMN_Data_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ODB_HPLMN_Data;
asn_struct_free_f ODB_HPLMN_Data_free;
asn_struct_print_f ODB_HPLMN_Data_print;
asn_constr_check_f ODB_HPLMN_Data_constraint;
ber_type_decoder_f ODB_HPLMN_Data_decode_ber;
der_type_encoder_f ODB_HPLMN_Data_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ODB_HPLMN_Data_decode_xer;
xer_type_encoder_f ODB_HPLMN_Data_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ODB_HPLMN_Data_H_ */
