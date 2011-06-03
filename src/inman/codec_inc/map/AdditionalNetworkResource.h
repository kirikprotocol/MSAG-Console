#ifndef	_AdditionalNetworkResource_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_AdditionalNetworkResource_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AdditionalNetworkResource {
	AdditionalNetworkResource_sgsn	= 0,
	AdditionalNetworkResource_ggsn	= 1,
	AdditionalNetworkResource_gmlc	= 2,
	AdditionalNetworkResource_gsmSCF	= 3,
	AdditionalNetworkResource_nplr	= 4,
	AdditionalNetworkResource_auc	= 5
	/*
	 * Enumeration is extensible
	 */
} AdditionalNetworkResource_e;

/* AdditionalNetworkResource */
typedef long	 AdditionalNetworkResource_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AdditionalNetworkResource;
asn_struct_free_f AdditionalNetworkResource_free;
asn_struct_print_f AdditionalNetworkResource_print;
asn_constr_check_f AdditionalNetworkResource_constraint;
ber_type_decoder_f AdditionalNetworkResource_decode_ber;
der_type_encoder_f AdditionalNetworkResource_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AdditionalNetworkResource_decode_xer;
xer_type_encoder_f AdditionalNetworkResource_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AdditionalNetworkResource_H_ */
