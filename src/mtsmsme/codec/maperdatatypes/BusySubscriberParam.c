#include <asn_internal.h>

#include "BusySubscriberParam.h"

static asn_TYPE_member_t asn_MBR_BusySubscriberParam_1[] = {
	{ ATF_POINTER, 3, offsetof(struct BusySubscriberParam, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct BusySubscriberParam, ccbs_Possible),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ccbs-Possible"
		},
	{ ATF_POINTER, 1, offsetof(struct BusySubscriberParam, ccbs_Busy),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ccbs-Busy"
		},
};
static ber_tlv_tag_t asn_DEF_BusySubscriberParam_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_BusySubscriberParam_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* extensionContainer at 203 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* ccbs-Possible at 205 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 } /* ccbs-Busy at 206 */
};
static asn_SEQUENCE_specifics_t asn_SPC_BusySubscriberParam_specs_1 = {
	sizeof(struct BusySubscriberParam),
	offsetof(struct BusySubscriberParam, _asn_ctx),
	asn_MAP_BusySubscriberParam_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	0,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_BusySubscriberParam = {
	"BusySubscriberParam",
	"BusySubscriberParam",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_BusySubscriberParam_tags_1,
	sizeof(asn_DEF_BusySubscriberParam_tags_1)
		/sizeof(asn_DEF_BusySubscriberParam_tags_1[0]), /* 1 */
	asn_DEF_BusySubscriberParam_tags_1,	/* Same as above */
	sizeof(asn_DEF_BusySubscriberParam_tags_1)
		/sizeof(asn_DEF_BusySubscriberParam_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_BusySubscriberParam_1,
	3,	/* Elements count */
	&asn_SPC_BusySubscriberParam_specs_1	/* Additional specs */
};

