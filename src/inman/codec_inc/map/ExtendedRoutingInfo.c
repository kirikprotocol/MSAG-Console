#ident "$Id$"

#include <asn_internal.h>

#include "ExtendedRoutingInfo.h"

static asn_TYPE_member_t asn_MBR_ExtendedRoutingInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ExtendedRoutingInfo, choice.routingInfo),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_RoutingInfo,
		0,	/* Defer constraints checking to the member type */
		"routingInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ExtendedRoutingInfo, choice.camelRoutingInfo),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelRoutingInfo,
		0,	/* Defer constraints checking to the member type */
		"camelRoutingInfo"
		},
};
static asn_TYPE_tag2member_t asn_MAP_ExtendedRoutingInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 0 }, /* roamingNumber at 188 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* forwardingData at 190 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 1, 0, 0 } /* camelRoutingInfo at 215 */
};
static asn_CHOICE_specifics_t asn_SPC_ExtendedRoutingInfo_1_specs = {
	sizeof(struct ExtendedRoutingInfo),
	offsetof(struct ExtendedRoutingInfo, _asn_ctx),
	offsetof(struct ExtendedRoutingInfo, present),
	sizeof(((struct ExtendedRoutingInfo *)0)->present),
	asn_MAP_ExtendedRoutingInfo_1_tag2el,
	3,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_ExtendedRoutingInfo = {
	"ExtendedRoutingInfo",
	"ExtendedRoutingInfo",
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
	asn_MBR_ExtendedRoutingInfo_1,
	2,	/* Elements count */
	&asn_SPC_ExtendedRoutingInfo_1_specs	/* Additional specs */
};

