#include <asn_internal.h>

#include "Invoke.h"

static asn_TYPE_member_t asn_MBR_Invoke_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Invoke, invokeID),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_InvokeIdType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invokeID"
		},
	{ ATF_POINTER, 1, offsetof(struct Invoke, linkedID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_InvokeIdType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"linkedID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Invoke, opcode),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Code,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"opcode"
		},
	{ ATF_OPEN_TYPE | ATF_POINTER, 1, offsetof(struct Invoke, argument),
		-1 /* Ambiguous tag (ANY?) */,
		0,
		&asn_DEF_ANY,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"argument"
		},
};
static ber_tlv_tag_t asn_DEF_Invoke_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Invoke_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 1 }, /* invokeID at 112 */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 2, -1, 0 }, /* local at 54 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 2, 0, 0 }, /* global at 55 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 } /* linkedID at 113 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Invoke_specs_1 = {
	sizeof(struct Invoke),
	offsetof(struct Invoke, _asn_ctx),
	asn_MAP_Invoke_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Invoke = {
	"Invoke",
	"Invoke",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Invoke_tags_1,
	sizeof(asn_DEF_Invoke_tags_1)
		/sizeof(asn_DEF_Invoke_tags_1[0]), /* 1 */
	asn_DEF_Invoke_tags_1,	/* Same as above */
	sizeof(asn_DEF_Invoke_tags_1)
		/sizeof(asn_DEF_Invoke_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Invoke_1,
	4,	/* Elements count */
	&asn_SPC_Invoke_specs_1	/* Additional specs */
};

