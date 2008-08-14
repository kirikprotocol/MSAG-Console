#include <asn_internal.h>

#include "LocationInfo.h"

static asn_TYPE_member_t asn_MBR_LocationInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct LocationInfo, choice.roamingNumber),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"roamingNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LocationInfo, choice.mscNumber),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mscNumber"
		},
};
static asn_TYPE_tag2member_t asn_MAP_LocationInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* roamingNumber at 28 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* mscNumber at 29 */
};
static asn_CHOICE_specifics_t asn_SPC_LocationInfo_specs_1 = {
	sizeof(struct LocationInfo),
	offsetof(struct LocationInfo, _asn_ctx),
	offsetof(struct LocationInfo, present),
	sizeof(((struct LocationInfo *)0)->present),
	asn_MAP_LocationInfo_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_LocationInfo = {
	"LocationInfo",
	"LocationInfo",
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
	asn_MBR_LocationInfo_1,
	2,	/* Elements count */
	&asn_SPC_LocationInfo_specs_1	/* Additional specs */
};

