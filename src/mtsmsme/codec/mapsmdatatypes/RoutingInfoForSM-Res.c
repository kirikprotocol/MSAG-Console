#include <asn_internal.h>

#include "RoutingInfoForSM-Res.h"

static asn_TYPE_member_t asn_MBR_RoutingInfoForSM_Res_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Res, imsi),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfoForSM_Res, locationInfoWithLMSI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInfoWithLMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"locationInfoWithLMSI"
		},
	{ ATF_POINTER, 1, offsetof(struct RoutingInfoForSM_Res, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_RoutingInfoForSM_Res_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RoutingInfoForSM_Res_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* imsi at 86 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* locationInfoWithLMSI at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 } /* extensionContainer at 88 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RoutingInfoForSM_Res_specs_1 = {
	sizeof(struct RoutingInfoForSM_Res),
	offsetof(struct RoutingInfoForSM_Res, _asn_ctx),
	asn_MAP_RoutingInfoForSM_Res_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RoutingInfoForSM_Res = {
	"RoutingInfoForSM-Res",
	"RoutingInfoForSM-Res",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_RoutingInfoForSM_Res_tags_1,
	sizeof(asn_DEF_RoutingInfoForSM_Res_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Res_tags_1[0]), /* 1 */
	asn_DEF_RoutingInfoForSM_Res_tags_1,	/* Same as above */
	sizeof(asn_DEF_RoutingInfoForSM_Res_tags_1)
		/sizeof(asn_DEF_RoutingInfoForSM_Res_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_RoutingInfoForSM_Res_1,
	3,	/* Elements count */
	&asn_SPC_RoutingInfoForSM_Res_specs_1	/* Additional specs */
};

