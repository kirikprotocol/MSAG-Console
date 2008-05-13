#include <asn_internal.h>

#include "MT-ForwardSM-Res.h"

static asn_TYPE_member_t asn_MBR_MT_ForwardSM_Res_1[] = {
	{ ATF_POINTER, 2, offsetof(struct MT_ForwardSM_Res, sm_RP_UI),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-UI"
		},
	{ ATF_POINTER, 1, offsetof(struct MT_ForwardSM_Res, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_MT_ForwardSM_Res_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MT_ForwardSM_Res_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* sm-RP-UI at 134 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensionContainer at 135 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MT_ForwardSM_Res_specs_1 = {
	sizeof(struct MT_ForwardSM_Res),
	offsetof(struct MT_ForwardSM_Res, _asn_ctx),
	asn_MAP_MT_ForwardSM_Res_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MT_ForwardSM_Res = {
	"MT-ForwardSM-Res",
	"MT-ForwardSM-Res",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MT_ForwardSM_Res_tags_1,
	sizeof(asn_DEF_MT_ForwardSM_Res_tags_1)
		/sizeof(asn_DEF_MT_ForwardSM_Res_tags_1[0]), /* 1 */
	asn_DEF_MT_ForwardSM_Res_tags_1,	/* Same as above */
	sizeof(asn_DEF_MT_ForwardSM_Res_tags_1)
		/sizeof(asn_DEF_MT_ForwardSM_Res_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MT_ForwardSM_Res_1,
	2,	/* Elements count */
	&asn_SPC_MT_ForwardSM_Res_specs_1	/* Additional specs */
};

