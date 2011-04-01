#include <asn_internal.h>

#include "SendRoutingInfoArg-v2.h"

static asn_TYPE_member_t asn_MBR_SendRoutingInfoArg_v2_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoArg_v2, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_POINTER, 3, offsetof(struct SendRoutingInfoArg_v2, cug_CheckInfo),
		(ber_tlv_tag_t)(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CUG_CheckInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cug-CheckInfo"
		},
	{ ATF_POINTER, 2, offsetof(struct SendRoutingInfoArg_v2, numberOfForwarding),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NumberOfForwarding,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"numberOfForwarding"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoArg_v2, networkSignalInfo),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkSignalInfo"
		},
};
static ber_tlv_tag_t asn_DEF_SendRoutingInfoArg_v2_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SendRoutingInfoArg_v2_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* msisdn at 74 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* cug-CheckInfo at 75 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* numberOfForwarding at 77 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 3, 0, 0 } /* networkSignalInfo at 78 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SendRoutingInfoArg_v2_specs_1 = {
	sizeof(struct SendRoutingInfoArg_v2),
	offsetof(struct SendRoutingInfoArg_v2, _asn_ctx),
	asn_MAP_SendRoutingInfoArg_v2_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	3,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoArg_v2 = {
	"SendRoutingInfoArg-v2",
	"SendRoutingInfoArg-v2",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SendRoutingInfoArg_v2_tags_1,
	sizeof(asn_DEF_SendRoutingInfoArg_v2_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoArg_v2_tags_1[0]), /* 1 */
	asn_DEF_SendRoutingInfoArg_v2_tags_1,	/* Same as above */
	sizeof(asn_DEF_SendRoutingInfoArg_v2_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoArg_v2_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SendRoutingInfoArg_v2_1,
	4,	/* Elements count */
	&asn_SPC_SendRoutingInfoArg_v2_specs_1	/* Additional specs */
};

