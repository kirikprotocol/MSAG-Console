#include <asn_internal.h>

#include "CamelInfo.h"

static asn_TYPE_member_t asn_MBR_CamelInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CamelInfo, supportedCamelPhases),
		(ASN_TAG_CLASS_UNIVERSAL | (3 << 2)),
		0,
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedCamelPhases"
		},
	{ ATF_POINTER, 3, offsetof(struct CamelInfo, suppress_T_CSI),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppress-T-CSI"
		},
	{ ATF_POINTER, 2, offsetof(struct CamelInfo, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct CamelInfo, offeredCamel4CSIs),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offeredCamel4CSIs"
		},
};
static ber_tlv_tag_t asn_DEF_CamelInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CamelInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (3 << 2)), 0, 0, 0 }, /* supportedCamelPhases at 233 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 }, /* suppress-T-CSI at 234 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 }, /* extensionContainer at 235 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 3, 0, 0 } /* offeredCamel4CSIs at 237 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CamelInfo_specs_1 = {
	sizeof(struct CamelInfo),
	offsetof(struct CamelInfo, _asn_ctx),
	asn_MAP_CamelInfo_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CamelInfo = {
	"CamelInfo",
	"CamelInfo",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CamelInfo_tags_1,
	sizeof(asn_DEF_CamelInfo_tags_1)
		/sizeof(asn_DEF_CamelInfo_tags_1[0]), /* 1 */
	asn_DEF_CamelInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_CamelInfo_tags_1)
		/sizeof(asn_DEF_CamelInfo_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CamelInfo_1,
	4,	/* Elements count */
	&asn_SPC_CamelInfo_specs_1	/* Additional specs */
};

