#ifndef	_NetworkResource_H_
#define	_NetworkResource_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum NetworkResource {
	NetworkResource_plmn	= 0,
	NetworkResource_hlr	= 1,
	NetworkResource_vlr	= 2,
	NetworkResource_pvlr	= 3,
	NetworkResource_controllingMSC	= 4,
	NetworkResource_vmsc	= 5,
	NetworkResource_eir	= 6,
	NetworkResource_rss	= 7
} NetworkResource_e;

/* NetworkResource */
typedef long	 NetworkResource_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NetworkResource;
asn_struct_free_f NetworkResource_free;
asn_struct_print_f NetworkResource_print;
asn_constr_check_f NetworkResource_constraint;
ber_type_decoder_f NetworkResource_decode_ber;
der_type_encoder_f NetworkResource_encode_der;
xer_type_decoder_f NetworkResource_decode_xer;
xer_type_encoder_f NetworkResource_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _NetworkResource_H_ */
