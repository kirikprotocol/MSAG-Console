#ident "$Id$"

#include <asn_internal.h>

#include "ChoiceType2.h"

static asn_TYPE_member_t asn_MBR_ChoiceType2_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ChoiceType2, choice.case1),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		"case1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ChoiceType2, choice.case2),
		(ASN_TAG_CLASS_UNIVERSAL | (1 << 2)),
		0,
		&asn_DEF_BOOLEAN,
		0,	/* Defer constraints checking to the member type */
		"case2"
		},
};
static ber_tlv_tag_t asn_DEF_ChoiceType2_1_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (15 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ChoiceType2_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (1 << 2)), 1, 0, 0 }, /* case2 at 46 */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 } /* case1 at 45 */
};
static asn_CHOICE_specifics_t asn_SPC_ChoiceType2_1_specs = {
	sizeof(struct ChoiceType2),
	offsetof(struct ChoiceType2, _asn_ctx),
	offsetof(struct ChoiceType2, present),
	sizeof(((struct ChoiceType2 *)0)->present),
	asn_MAP_ChoiceType2_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_ChoiceType2 = {
	"ChoiceType2",
	"ChoiceType2",
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
	asn_DEF_ChoiceType2_1_tags,
	sizeof(asn_DEF_ChoiceType2_1_tags)
		/sizeof(asn_DEF_ChoiceType2_1_tags[0]), /* 1 */
	asn_DEF_ChoiceType2_1_tags,	/* Same as above */
	sizeof(asn_DEF_ChoiceType2_1_tags)
		/sizeof(asn_DEF_ChoiceType2_1_tags[0]), /* 1 */
	asn_MBR_ChoiceType2_1,
	2,	/* Elements count */
	&asn_SPC_ChoiceType2_1_specs	/* Additional specs */
};

