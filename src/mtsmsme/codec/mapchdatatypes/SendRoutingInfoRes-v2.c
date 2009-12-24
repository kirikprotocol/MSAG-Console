#include <asn_internal.h>

#include "SendRoutingInfoRes-v2.h"

static asn_TYPE_member_t asn_MBR_SendRoutingInfoRes_v2_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoRes_v2, imsi),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoRes_v2, routingInfo),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_RoutingInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routingInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoRes_v2, cug_CheckInfo),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_CUG_CheckInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cug-CheckInfo"
		},
};
static ber_tlv_tag_t asn_DEF_SendRoutingInfoRes_v2_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SendRoutingInfoRes_v2_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 1 }, /* imsi at 139 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 0 }, /* roamingNumber at 204 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 1 }, /* forwardingData at 205 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, -1, 0 } /* cug-CheckInfo at 141 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SendRoutingInfoRes_v2_specs_1 = {
	sizeof(struct SendRoutingInfoRes_v2),
	offsetof(struct SendRoutingInfoRes_v2, _asn_ctx),
	asn_MAP_SendRoutingInfoRes_v2_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoRes_v2 = {
	"SendRoutingInfoRes-v2",
	"SendRoutingInfoRes-v2",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SendRoutingInfoRes_v2_tags_1,
	sizeof(asn_DEF_SendRoutingInfoRes_v2_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoRes_v2_tags_1[0]), /* 1 */
	asn_DEF_SendRoutingInfoRes_v2_tags_1,	/* Same as above */
	sizeof(asn_DEF_SendRoutingInfoRes_v2_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoRes_v2_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SendRoutingInfoRes_v2_1,
	3,	/* Elements count */
	&asn_SPC_SendRoutingInfoRes_v2_specs_1	/* Additional specs */
};

