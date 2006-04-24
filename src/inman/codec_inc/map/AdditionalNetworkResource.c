#ident "$Id$"

#include <asn_internal.h>

#include "AdditionalNetworkResource.h"

int
AdditionalNetworkResource_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
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
AdditionalNetworkResource_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
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
AdditionalNetworkResource_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
AdditionalNetworkResource_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
AdditionalNetworkResource_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
AdditionalNetworkResource_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

#ifndef ASN1_XER_NOT_USED
asn_dec_rval_t
AdditionalNetworkResource_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
AdditionalNetworkResource_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	AdditionalNetworkResource_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

#endif /* ASN1_XER_NOT_USED */
static asn_INTEGER_enum_map_t asn_MAP_AdditionalNetworkResource_1_value2enum[] = {
	{ 0,	4,	"sgsn" },
	{ 1,	4,	"ggsn" },
	{ 2,	4,	"gmlc" },
	{ 3,	6,	"gsmSCF" },
	{ 4,	4,	"nplr" },
	{ 5,	3,	"auc" }
	/* This list is extensible */
};
static unsigned int asn_MAP_AdditionalNetworkResource_1_enum2value[] = {
	5,	/* auc(5) */
	1,	/* ggsn(1) */
	2,	/* gmlc(2) */
	3,	/* gsmSCF(3) */
	4,	/* nplr(4) */
	0	/* sgsn(0) */
	/* This list is extensible */
};
static asn_INTEGER_specifics_t asn_SPC_AdditionalNetworkResource_1_specs = {
	asn_MAP_AdditionalNetworkResource_1_value2enum,	/* "tag" => N; sorted by tag */
	asn_MAP_AdditionalNetworkResource_1_enum2value,	/* N => "tag"; sorted by N */
	6,	/* Number of elements in the maps */
	1,	/* Enumeration is extensible */
	1	/* Strict enumeration */
};
static ber_tlv_tag_t asn_DEF_AdditionalNetworkResource_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_AdditionalNetworkResource = {
	"AdditionalNetworkResource",
	"AdditionalNetworkResource",
	AdditionalNetworkResource_free,
	AdditionalNetworkResource_print,
	AdditionalNetworkResource_constraint,
	AdditionalNetworkResource_decode_ber,
	AdditionalNetworkResource_encode_der,
#ifndef ASN1_XER_NOT_USED
	AdditionalNetworkResource_decode_xer,
	AdditionalNetworkResource_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_AdditionalNetworkResource_1_tags,
	sizeof(asn_DEF_AdditionalNetworkResource_1_tags)
		/sizeof(asn_DEF_AdditionalNetworkResource_1_tags[0]), /* 1 */
	asn_DEF_AdditionalNetworkResource_1_tags,	/* Same as above */
	sizeof(asn_DEF_AdditionalNetworkResource_1_tags)
		/sizeof(asn_DEF_AdditionalNetworkResource_1_tags[0]), /* 1 */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_AdditionalNetworkResource_1_specs	/* Additional specs */
};

