#include <asn_internal.h>

#include "ExtendedRoutingInfo.h"

static asn_TYPE_member_t asn_MBR_ExtendedRoutingInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ExtendedRoutingInfo, choice.routingInfo),
		(ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_RoutingInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routingInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ExtendedRoutingInfo, choice.camelRoutingInfo),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelRoutingInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"camelRoutingInfo"
		},
};
static asn_TYPE_tag2member_t asn_MAP_ExtendedRoutingInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* roamingNumber at 188 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* forwardingData at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 1, 0, 0 } /* camelRoutingInfo at 242 */
};
static asn_CHOICE_specifics_t asn_SPC_ExtendedRoutingInfo_specs_1 = {
	sizeof(struct ExtendedRoutingInfo),
	offsetof(struct ExtendedRoutingInfo, _asn_ctx),
	offsetof(struct ExtendedRoutingInfo, present),
	sizeof(((struct ExtendedRoutingInfo *)0)->present),
	asn_MAP_ExtendedRoutingInfo_tag2el_1,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_ExtendedRoutingInfo = {
	"ExtendedRoutingInfo",
	"ExtendedRoutingInfo",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_ExtendedRoutingInfo_1,
	2,	/* Elements count */
	&asn_SPC_ExtendedRoutingInfo_specs_1	/* Additional specs */
};

