#include <asn_internal.h>

#include "MT-ForwardSM-VGCS-Res.h"

static asn_TYPE_member_t asn_MBR_MT_ForwardSM_VGCS_Res_1[] = {
	{ ATF_POINTER, 4, offsetof(struct MT_ForwardSM_VGCS_Res, sm_RP_UI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-UI"
		},
	{ ATF_POINTER, 3, offsetof(struct MT_ForwardSM_VGCS_Res, dispatcherList),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DispatcherList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dispatcherList"
		},
	{ ATF_POINTER, 2, offsetof(struct MT_ForwardSM_VGCS_Res, ongoingCall),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ongoingCall"
		},
	{ ATF_POINTER, 1, offsetof(struct MT_ForwardSM_VGCS_Res, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_MT_ForwardSM_VGCS_Res_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MT_ForwardSM_VGCS_Res_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 2, 0, 0 }, /* ongoingCall at 234 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sm-RP-UI at 232 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* dispatcherList at 233 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 } /* extensionContainer at 235 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MT_ForwardSM_VGCS_Res_specs_1 = {
	sizeof(struct MT_ForwardSM_VGCS_Res),
	offsetof(struct MT_ForwardSM_VGCS_Res, _asn_ctx),
	asn_MAP_MT_ForwardSM_VGCS_Res_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MT_ForwardSM_VGCS_Res = {
	"MT-ForwardSM-VGCS-Res",
	"MT-ForwardSM-VGCS-Res",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MT_ForwardSM_VGCS_Res_tags_1,
	sizeof(asn_DEF_MT_ForwardSM_VGCS_Res_tags_1)
		/sizeof(asn_DEF_MT_ForwardSM_VGCS_Res_tags_1[0]), /* 1 */
	asn_DEF_MT_ForwardSM_VGCS_Res_tags_1,	/* Same as above */
	sizeof(asn_DEF_MT_ForwardSM_VGCS_Res_tags_1)
		/sizeof(asn_DEF_MT_ForwardSM_VGCS_Res_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MT_ForwardSM_VGCS_Res_1,
	4,	/* Elements count */
	&asn_SPC_MT_ForwardSM_VGCS_Res_specs_1	/* Additional specs */
};

