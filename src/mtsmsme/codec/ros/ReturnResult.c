#include <asn_internal.h>

#include "ReturnResult.h"

static asn_TYPE_member_t asn_MBR_result_3[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct result, opcode),
	  (ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Code,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"opcode"
		},
	{ ATF_OPEN_TYPE | ATF_NOFLAGS, 0, offsetof(struct result, result),
	  (ber_tlv_tag_t)-1 /* Ambiguous tag (ANY?) */,
		0,
		&asn_DEF_ANY,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"result"
		},
};
static ber_tlv_tag_t asn_DEF_result_tags_3[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_result_tag2el_3[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* local at 114 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 } /* global at 115 */
};
static asn_SEQUENCE_specifics_t asn_SPC_result_specs_3 = {
	sizeof(struct result),
	offsetof(struct result, _asn_ctx),
	asn_MAP_result_tag2el_3,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_result_3 = {
	"result",
	"result",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_result_tags_3,
	sizeof(asn_DEF_result_tags_3)
		/sizeof(asn_DEF_result_tags_3[0]), /* 1 */
	asn_DEF_result_tags_3,	/* Same as above */
	sizeof(asn_DEF_result_tags_3)
		/sizeof(asn_DEF_result_tags_3[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_result_3,
	2,	/* Elements count */
	&asn_SPC_result_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ReturnResult_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ReturnResult, invokeId),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_InvokeId,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invokeId"
		},
	{ ATF_POINTER, 1, offsetof(struct ReturnResult, result),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_result_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"result"
		},
};
static ber_tlv_tag_t asn_DEF_ReturnResult_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ReturnResult_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* invokeId at 65 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* result at 71 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ReturnResult_specs_1 = {
	sizeof(struct ReturnResult),
	offsetof(struct ReturnResult, _asn_ctx),
	asn_MAP_ReturnResult_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ReturnResult = {
	"ReturnResult",
	"ReturnResult",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ReturnResult_tags_1,
	sizeof(asn_DEF_ReturnResult_tags_1)
		/sizeof(asn_DEF_ReturnResult_tags_1[0]), /* 1 */
	asn_DEF_ReturnResult_tags_1,	/* Same as above */
	sizeof(asn_DEF_ReturnResult_tags_1)
		/sizeof(asn_DEF_ReturnResult_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ReturnResult_1,
	2,	/* Elements count */
	&asn_SPC_ReturnResult_specs_1	/* Additional specs */
};

