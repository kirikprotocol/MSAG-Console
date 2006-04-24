#ident "$Id$"

#include <asn_internal.h>

#include "Ext-ForwFeature.h"

static asn_TYPE_member_t asn_MBR_Ext_ForwFeature_1[] = {
	{ ATF_POINTER, 1, offsetof(struct Ext_ForwFeature, basicService),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		"basicService"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Ext_ForwFeature, ss_Status),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_SS_Status,
		0,	/* Defer constraints checking to the member type */
		"ss-Status"
		},
	{ ATF_POINTER, 6, offsetof(struct Ext_ForwFeature, forwardedToNumber),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"forwardedToNumber"
		},
	{ ATF_POINTER, 5, offsetof(struct Ext_ForwFeature, forwardedToSubaddress),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_SubaddressString,
		0,	/* Defer constraints checking to the member type */
		"forwardedToSubaddress"
		},
	{ ATF_POINTER, 4, offsetof(struct Ext_ForwFeature, forwardingOptions),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_ForwOptions,
		0,	/* Defer constraints checking to the member type */
		"forwardingOptions"
		},
	{ ATF_POINTER, 3, offsetof(struct Ext_ForwFeature, noReplyConditionTime),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_NoRepCondTime,
		0,	/* Defer constraints checking to the member type */
		"noReplyConditionTime"
		},
	{ ATF_POINTER, 2, offsetof(struct Ext_ForwFeature, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct Ext_ForwFeature, longForwardedToNumber),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FTN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"longForwardedToNumber"
		},
};
static ber_tlv_tag_t asn_DEF_Ext_ForwFeature_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Ext_ForwFeature_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* ext-BearerService at 267 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 0, 0, 0 }, /* ext-Teleservice at 269 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, 0, 0 }, /* ss-Status at 326 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 2, 0, 0 }, /* forwardedToNumber at 327 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 4, 0, 0 }, /* forwardingOptions at 332 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 5, 0, 0 }, /* noReplyConditionTime at 333 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 3, 0, 0 }, /* forwardedToSubaddress at 331 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 6, 0, 0 }, /* extensionContainer at 334 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 7, 0, 0 } /* longForwardedToNumber at 336 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Ext_ForwFeature_1_specs = {
	sizeof(struct Ext_ForwFeature),
	offsetof(struct Ext_ForwFeature, _asn_ctx),
	asn_MAP_Ext_ForwFeature_1_tag2el,
	9,	/* Count of tags in the map */
	6,	/* Start extensions */
	9	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Ext_ForwFeature = {
	"Ext-ForwFeature",
	"Ext-ForwFeature",
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
	asn_DEF_Ext_ForwFeature_1_tags,
	sizeof(asn_DEF_Ext_ForwFeature_1_tags)
		/sizeof(asn_DEF_Ext_ForwFeature_1_tags[0]), /* 1 */
	asn_DEF_Ext_ForwFeature_1_tags,	/* Same as above */
	sizeof(asn_DEF_Ext_ForwFeature_1_tags)
		/sizeof(asn_DEF_Ext_ForwFeature_1_tags[0]), /* 1 */
	asn_MBR_Ext_ForwFeature_1,
	8,	/* Elements count */
	&asn_SPC_Ext_ForwFeature_1_specs	/* Additional specs */
};

