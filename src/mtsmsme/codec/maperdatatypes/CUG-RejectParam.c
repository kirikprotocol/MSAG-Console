#include <asn_internal.h>

#include "CUG-RejectParam.h"

static asn_TYPE_member_t asn_MBR_CUG_RejectParam_1[] = {
	{ ATF_POINTER, 2, offsetof(struct CUG_RejectParam, cug_RejectCause),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CUG_RejectCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cug-RejectCause"
		},
	{ ATF_POINTER, 1, offsetof(struct CUG_RejectParam, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CUG_RejectParam_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CUG_RejectParam_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* cug-RejectCause at 99 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensionContainer at 100 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CUG_RejectParam_specs_1 = {
	sizeof(struct CUG_RejectParam),
	offsetof(struct CUG_RejectParam, _asn_ctx),
	asn_MAP_CUG_RejectParam_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CUG_RejectParam = {
	"CUG-RejectParam",
	"CUG-RejectParam",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CUG_RejectParam_tags_1,
	sizeof(asn_DEF_CUG_RejectParam_tags_1)
		/sizeof(asn_DEF_CUG_RejectParam_tags_1[0]), /* 1 */
	asn_DEF_CUG_RejectParam_tags_1,	/* Same as above */
	sizeof(asn_DEF_CUG_RejectParam_tags_1)
		/sizeof(asn_DEF_CUG_RejectParam_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CUG_RejectParam_1,
	2,	/* Elements count */
	&asn_SPC_CUG_RejectParam_specs_1	/* Additional specs */
};

