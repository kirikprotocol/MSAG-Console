#ident "$Id$"

#include <asn_internal.h>

#include "RoutingInfo.h"

static asn_TYPE_member_t asn_MBR_RoutingInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfo, choice.roamingNumber),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"roamingNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct RoutingInfo, choice.forwardingData),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ForwardingData,
		0,	/* Defer constraints checking to the member type */
		"forwardingData"
		},
};
static asn_TYPE_tag2member_t asn_MAP_RoutingInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* roamingNumber at 188 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* forwardingData at 190 */
};
static asn_CHOICE_specifics_t asn_SPC_RoutingInfo_1_specs = {
	sizeof(struct RoutingInfo),
	offsetof(struct RoutingInfo, _asn_ctx),
	offsetof(struct RoutingInfo, present),
	sizeof(((struct RoutingInfo *)0)->present),
	asn_MAP_RoutingInfo_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_RoutingInfo = {
	"RoutingInfo",
	"RoutingInfo",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
#ifndef ASN1_XER_NOT_USED
	CHOICE_decode_xer,
	CHOICE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_RoutingInfo_1,
	2,	/* Elements count */
	&asn_SPC_RoutingInfo_1_specs	/* Additional specs */
};

