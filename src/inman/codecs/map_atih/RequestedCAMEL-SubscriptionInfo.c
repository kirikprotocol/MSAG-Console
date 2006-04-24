#ident "$Id$"

#include <asn_internal.h>

#include "RequestedCAMEL-SubscriptionInfo.h"

int
RequestedCAMEL_SubscriptionInfo_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, app_errlog, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
#ifndef ASN1_XER_NOT_USED
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
#else /* ASN1_XER_NOT_USED */
	td->xer_decoder    = NULL;
	td->xer_encoder    = NULL;
#endif /* ASN1_XER_NOT_USED */
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

void
RequestedCAMEL_SubscriptionInfo_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
RequestedCAMEL_SubscriptionInfo_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
RequestedCAMEL_SubscriptionInfo_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
RequestedCAMEL_SubscriptionInfo_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

#ifndef ASN1_XER_NOT_USED
asn_dec_rval_t
RequestedCAMEL_SubscriptionInfo_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
RequestedCAMEL_SubscriptionInfo_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	RequestedCAMEL_SubscriptionInfo_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

#endif /* ASN1_XER_NOT_USED */
static asn_INTEGER_enum_map_t asn_MAP_RequestedCAMEL_SubscriptionInfo_1_value2enum[] = {
	{ 0,	5,	"o-CSI" },
	{ 1,	5,	"t-CSI" },
	{ 2,	6,	"vt-CSI" },
	{ 3,	7,	"tif-CSI" },
	{ 4,	8,	"gprs-CSI" },
	{ 5,	10,	"mo-sms-CSI" },
	{ 6,	6,	"ss-CSI" },
	{ 7,	5,	"m-CSI" },
	{ 8,	5,	"d-csi" }
};
static unsigned int asn_MAP_RequestedCAMEL_SubscriptionInfo_1_enum2value[] = {
	8,	/* d-csi(8) */
	4,	/* gprs-CSI(4) */
	7,	/* m-CSI(7) */
	5,	/* mo-sms-CSI(5) */
	0,	/* o-CSI(0) */
	6,	/* ss-CSI(6) */
	1,	/* t-CSI(1) */
	3,	/* tif-CSI(3) */
	2	/* vt-CSI(2) */
};
static asn_INTEGER_specifics_t asn_SPC_RequestedCAMEL_SubscriptionInfo_1_specs = {
	asn_MAP_RequestedCAMEL_SubscriptionInfo_1_value2enum,	/* "tag" => N; sorted by tag */
	asn_MAP_RequestedCAMEL_SubscriptionInfo_1_enum2value,	/* N => "tag"; sorted by N */
	9,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1	/* Strict enumeration */
};
static ber_tlv_tag_t asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_RequestedCAMEL_SubscriptionInfo = {
	"RequestedCAMEL-SubscriptionInfo",
	"RequestedCAMEL-SubscriptionInfo",
	RequestedCAMEL_SubscriptionInfo_free,
	RequestedCAMEL_SubscriptionInfo_print,
	RequestedCAMEL_SubscriptionInfo_constraint,
	RequestedCAMEL_SubscriptionInfo_decode_ber,
	RequestedCAMEL_SubscriptionInfo_encode_der,
#ifndef ASN1_XER_NOT_USED
	RequestedCAMEL_SubscriptionInfo_decode_xer,
	RequestedCAMEL_SubscriptionInfo_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags,
	sizeof(asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags)
		/sizeof(asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags[0]), /* 1 */
	asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags)
		/sizeof(asn_DEF_RequestedCAMEL_SubscriptionInfo_1_tags[0]), /* 1 */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_RequestedCAMEL_SubscriptionInfo_1_specs	/* Additional specs */
};

