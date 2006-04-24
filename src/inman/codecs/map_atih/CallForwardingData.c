#ident "$Id$"

#include <asn_internal.h>

#include "CallForwardingData.h"

static asn_TYPE_member_t asn_MBR_CallForwardingData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CallForwardingData, forwardingFeatureList),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_Ext_ForwFeatureList,
		0,	/* Defer constraints checking to the member type */
		"forwardingFeatureList"
		},
	{ ATF_POINTER, 2, offsetof(struct CallForwardingData, notificationToCSE),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct CallForwardingData, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CallForwardingData_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CallForwardingData_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 }, /* notificationToCSE at 811 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* forwardingFeatureList at 810 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 } /* extensionContainer at 812 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CallForwardingData_1_specs = {
	sizeof(struct CallForwardingData),
	offsetof(struct CallForwardingData, _asn_ctx),
	asn_MAP_CallForwardingData_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CallForwardingData = {
	"CallForwardingData",
	"CallForwardingData",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_CallForwardingData_1_tags,
	sizeof(asn_DEF_CallForwardingData_1_tags)
		/sizeof(asn_DEF_CallForwardingData_1_tags[0]), /* 1 */
	asn_DEF_CallForwardingData_1_tags,	/* Same as above */
	sizeof(asn_DEF_CallForwardingData_1_tags)
		/sizeof(asn_DEF_CallForwardingData_1_tags[0]), /* 1 */
	asn_MBR_CallForwardingData_1,
	3,	/* Elements count */
	&asn_SPC_CallForwardingData_1_specs	/* Additional specs */
};

