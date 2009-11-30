#include <asn_internal.h>

#include "RoutingInfo.h"

static asn_TYPE_member_t asn_MBR_RoutingInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfo, choice.roamingNumber),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"roamingNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfo, choice.forwardingData),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ForwardingData,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forwardingData"
		},
};
static asn_TYPE_tag2member_t asn_MAP_RoutingInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* roamingNumber at 188 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* forwardingData at 189 */
};
static asn_CHOICE_specifics_t asn_SPC_RoutingInfo_specs_1 = {
	sizeof(struct RoutingInfo),
	offsetof(struct RoutingInfo, _asn_ctx),
	offsetof(struct RoutingInfo, present),
	sizeof(((struct RoutingInfo *)0)->present),
	asn_MAP_RoutingInfo_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_RoutingInfo = {
	"RoutingInfo",
	"RoutingInfo",
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
	asn_MBR_RoutingInfo_1,
	2,	/* Elements count */
	&asn_SPC_RoutingInfo_specs_1	/* Additional specs */
};

