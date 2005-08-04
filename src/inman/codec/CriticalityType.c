#include <asn_internal.h>

#include "CriticalityType.h"

int
CriticalityType_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
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
CriticalityType_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

void
CriticalityType_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
CriticalityType_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
CriticalityType_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
CriticalityType_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
CriticalityType_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
CriticalityType_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	CriticalityType_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_INTEGER_enum_map_t asn_MAP_CriticalityType_1_value2enum[] = {
	{ 0,	6,	"ignore" },
	{ 1,	5,	"abort" }
};
static unsigned int asn_MAP_CriticalityType_1_enum2value[] = {
	1,	/* abort(1) */
	0	/* ignore(0) */
};
static asn_INTEGER_specifics_t asn_SPC_CriticalityType_1_specs = {
	asn_MAP_CriticalityType_1_value2enum,	/* "tag" => N; sorted by tag */
	asn_MAP_CriticalityType_1_enum2value,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1	/* Strict enumeration */
};
static ber_tlv_tag_t asn_DEF_CriticalityType_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_CriticalityType = {
	"CriticalityType",
	"CriticalityType",
	CriticalityType_free,
	CriticalityType_print,
	CriticalityType_constraint,
	CriticalityType_decode_ber,
	CriticalityType_encode_der,
	CriticalityType_decode_xer,
	CriticalityType_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CriticalityType_1_tags,
	sizeof(asn_DEF_CriticalityType_1_tags)
		/sizeof(asn_DEF_CriticalityType_1_tags[0]), /* 1 */
	asn_DEF_CriticalityType_1_tags,	/* Same as above */
	sizeof(asn_DEF_CriticalityType_1_tags)
		/sizeof(asn_DEF_CriticalityType_1_tags[0]), /* 1 */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_CriticalityType_1_specs	/* Additional specs */
};

