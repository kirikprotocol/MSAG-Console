#include <asn_internal.h>

#include "CamelRoutingInfo.h"

static asn_TYPE_member_t asn_MBR_CamelRoutingInfo_1[] = {
	{ ATF_POINTER, 1, offsetof(struct CamelRoutingInfo, forwardingData),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ForwardingData,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forwardingData"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CamelRoutingInfo, gmscCamelSubscriptionInfo),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GmscCamelSubscriptionInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gmscCamelSubscriptionInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct CamelRoutingInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CamelRoutingInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CamelRoutingInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* forwardingData at 245 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* gmscCamelSubscriptionInfo at 246 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* extensionContainer at 247 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CamelRoutingInfo_specs_1 = {
	sizeof(struct CamelRoutingInfo),
	offsetof(struct CamelRoutingInfo, _asn_ctx),
	asn_MAP_CamelRoutingInfo_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CamelRoutingInfo = {
	"CamelRoutingInfo",
	"CamelRoutingInfo",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CamelRoutingInfo_tags_1,
	sizeof(asn_DEF_CamelRoutingInfo_tags_1)
		/sizeof(asn_DEF_CamelRoutingInfo_tags_1[0]), /* 1 */
	asn_DEF_CamelRoutingInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_CamelRoutingInfo_tags_1)
		/sizeof(asn_DEF_CamelRoutingInfo_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_CamelRoutingInfo_1,
	3,	/* Elements count */
	&asn_SPC_CamelRoutingInfo_specs_1	/* Additional specs */
};

