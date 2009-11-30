#include <asn_internal.h>

#include "GPRSMSClass.h"

static asn_TYPE_member_t asn_MBR_GPRSMSClass_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GPRSMSClass, mSNetworkCapability),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MSNetworkCapability,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mSNetworkCapability"
		},
	{ ATF_POINTER, 1, offsetof(struct GPRSMSClass, mSRadioAccessCapability),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MSRadioAccessCapability,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mSRadioAccessCapability"
		},
};
static ber_tlv_tag_t asn_DEF_GPRSMSClass_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_GPRSMSClass_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* mSNetworkCapability at 483 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* mSRadioAccessCapability at 484 */
};
static asn_SEQUENCE_specifics_t asn_SPC_GPRSMSClass_specs_1 = {
	sizeof(struct GPRSMSClass),
	offsetof(struct GPRSMSClass, _asn_ctx),
	asn_MAP_GPRSMSClass_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_GPRSMSClass = {
	"GPRSMSClass",
	"GPRSMSClass",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_GPRSMSClass_tags_1,
	sizeof(asn_DEF_GPRSMSClass_tags_1)
		/sizeof(asn_DEF_GPRSMSClass_tags_1[0]), /* 1 */
	asn_DEF_GPRSMSClass_tags_1,	/* Same as above */
	sizeof(asn_DEF_GPRSMSClass_tags_1)
		/sizeof(asn_DEF_GPRSMSClass_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_GPRSMSClass_1,
	2,	/* Elements count */
	&asn_SPC_GPRSMSClass_specs_1	/* Additional specs */
};

