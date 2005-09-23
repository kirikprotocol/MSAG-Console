#ifndef	_AppendFreeFormatData_H_
#define	_AppendFreeFormatData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AppendFreeFormatData {
	AppendFreeFormatData_overwrite	= 0,
	AppendFreeFormatData_append	= 1
} AppendFreeFormatData_e;

/* AppendFreeFormatData */
typedef long	 AppendFreeFormatData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AppendFreeFormatData;
asn_struct_free_f AppendFreeFormatData_free;
asn_struct_print_f AppendFreeFormatData_print;
asn_constr_check_f AppendFreeFormatData_constraint;
ber_type_decoder_f AppendFreeFormatData_decode_ber;
der_type_encoder_f AppendFreeFormatData_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AppendFreeFormatData_decode_xer;
xer_type_encoder_f AppendFreeFormatData_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AppendFreeFormatData_H_ */
