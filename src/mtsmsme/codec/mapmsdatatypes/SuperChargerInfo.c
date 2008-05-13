#include <asn_internal.h>

#include "SuperChargerInfo.h"

static asn_TYPE_member_t asn_MBR_SuperChargerInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SuperChargerInfo, choice.sendSubscriberData),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sendSubscriberData"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SuperChargerInfo, choice.subscriberDataStored),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AgeIndicator,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"subscriberDataStored"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SuperChargerInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sendSubscriberData at 76 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* subscriberDataStored at 78 */
};
static asn_CHOICE_specifics_t asn_SPC_SuperChargerInfo_specs_1 = {
	sizeof(struct SuperChargerInfo),
	offsetof(struct SuperChargerInfo, _asn_ctx),
	offsetof(struct SuperChargerInfo, present),
	sizeof(((struct SuperChargerInfo *)0)->present),
	asn_MAP_SuperChargerInfo_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SuperChargerInfo = {
	"SuperChargerInfo",
	"SuperChargerInfo",
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
	asn_MBR_SuperChargerInfo_1,
	2,	/* Elements count */
	&asn_SPC_SuperChargerInfo_specs_1	/* Additional specs */
};

