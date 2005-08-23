#include <asn_internal.h>

#include "MiscCallInfo.h"

static int
messageType_2_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
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
messageType_2_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
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

static void
messageType_2_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	messageType_2_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

static int
messageType_2_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	messageType_2_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

static asn_dec_rval_t
messageType_2_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	messageType_2_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

static asn_enc_rval_t
messageType_2_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	messageType_2_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

static asn_dec_rval_t
messageType_2_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	messageType_2_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

static asn_enc_rval_t
messageType_2_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	messageType_2_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_INTEGER_enum_map_t asn_MAP_messageType_2_value2enum[] = {
	{ 0,	7,	"request" },
	{ 1,	12,	"notification" }
	/* This list is extensible */
};
static unsigned int asn_MAP_messageType_2_enum2value[] = {
	1,	/* notification(1) */
	0	/* request(0) */
	/* This list is extensible */
};
static asn_INTEGER_specifics_t asn_SPC_messageType_2_specs = {
	asn_MAP_messageType_2_value2enum,	/* "tag" => N; sorted by tag */
	asn_MAP_messageType_2_enum2value,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	1,	/* Enumeration is extensible */
	1	/* Strict enumeration */
};
static ber_tlv_tag_t asn_DEF_messageType_2_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_messageType_2 = {
	"messageType",
	"messageType",
	messageType_2_free,
	messageType_2_print,
	messageType_2_constraint,
	messageType_2_decode_ber,
	messageType_2_encode_der,
	messageType_2_decode_xer,
	messageType_2_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_messageType_2_tags,
	sizeof(asn_DEF_messageType_2_tags)
		/sizeof(asn_DEF_messageType_2_tags[0]) - 1, /* 1 */
	asn_DEF_messageType_2_tags,	/* Same as above */
	sizeof(asn_DEF_messageType_2_tags)
		/sizeof(asn_DEF_messageType_2_tags[0]), /* 2 */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_messageType_2_specs	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_MiscCallInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MiscCallInfo, messageType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_messageType_2,
		0,	/* Defer constraints checking to the member type */
		"messageType"
		},
};
static ber_tlv_tag_t asn_DEF_MiscCallInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MiscCallInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* messageType at 166 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MiscCallInfo_1_specs = {
	sizeof(struct MiscCallInfo),
	offsetof(struct MiscCallInfo, _asn_ctx),
	asn_MAP_MiscCallInfo_1_tag2el,
	1,	/* Count of tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MiscCallInfo = {
	"MiscCallInfo",
	"MiscCallInfo",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MiscCallInfo_1_tags,
	sizeof(asn_DEF_MiscCallInfo_1_tags)
		/sizeof(asn_DEF_MiscCallInfo_1_tags[0]), /* 1 */
	asn_DEF_MiscCallInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_MiscCallInfo_1_tags)
		/sizeof(asn_DEF_MiscCallInfo_1_tags[0]), /* 1 */
	asn_MBR_MiscCallInfo_1,
	1,	/* Elements count */
	&asn_SPC_MiscCallInfo_1_specs	/* Additional specs */
};

