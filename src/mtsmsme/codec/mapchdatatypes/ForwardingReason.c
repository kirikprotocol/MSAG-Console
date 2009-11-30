#include <asn_internal.h>

#include "ForwardingReason.h"

int
ForwardingReason_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
ForwardingReason_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

void
ForwardingReason_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
ForwardingReason_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
ForwardingReason_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
ForwardingReason_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
ForwardingReason_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
ForwardingReason_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	ForwardingReason_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_INTEGER_enum_map_t asn_MAP_ForwardingReason_value2enum_1[] = {
	{ 0,	12,	"notReachable" },
	{ 1,	4,	"busy" },
	{ 2,	7,	"noReply" }
};
static unsigned int asn_MAP_ForwardingReason_enum2value_1[] = {
	1,	/* busy(1) */
	2,	/* noReply(2) */
	0	/* notReachable(0) */
};
static asn_INTEGER_specifics_t asn_SPC_ForwardingReason_specs_1 = {
	asn_MAP_ForwardingReason_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_ForwardingReason_enum2value_1,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1	/* Strict enumeration */
};
static ber_tlv_tag_t asn_DEF_ForwardingReason_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_ForwardingReason = {
	"ForwardingReason",
	"ForwardingReason",
	ForwardingReason_free,
	ForwardingReason_print,
	ForwardingReason_constraint,
	ForwardingReason_decode_ber,
	ForwardingReason_encode_der,
	ForwardingReason_decode_xer,
	ForwardingReason_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ForwardingReason_tags_1,
	sizeof(asn_DEF_ForwardingReason_tags_1)
		/sizeof(asn_DEF_ForwardingReason_tags_1[0]), /* 1 */
	asn_DEF_ForwardingReason_tags_1,	/* Same as above */
	sizeof(asn_DEF_ForwardingReason_tags_1)
		/sizeof(asn_DEF_ForwardingReason_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_ForwardingReason_specs_1	/* Additional specs */
};

