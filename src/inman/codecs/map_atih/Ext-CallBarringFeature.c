#ident "$Id$"

#include <asn_internal.h>

#include "Ext-CallBarringFeature.h"

static asn_TYPE_member_t asn_MBR_Ext_CallBarringFeature_1[] = {
	{ ATF_POINTER, 1, offsetof(struct Ext_CallBarringFeature, basicService),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		"basicService"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Ext_CallBarringFeature, ss_Status),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_SS_Status,
		0,	/* Defer constraints checking to the member type */
		"ss-Status"
		},
	{ ATF_POINTER, 1, offsetof(struct Ext_CallBarringFeature, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_Ext_CallBarringFeature_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Ext_CallBarringFeature_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 }, /* extensionContainer at 374 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* ext-BearerService at 267 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 0, 0, 0 }, /* ext-Teleservice at 269 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, 0, 0 } /* ss-Status at 373 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Ext_CallBarringFeature_1_specs = {
	sizeof(struct Ext_CallBarringFeature),
	offsetof(struct Ext_CallBarringFeature, _asn_ctx),
	asn_MAP_Ext_CallBarringFeature_1_tag2el,
	4,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Ext_CallBarringFeature = {
	"Ext-CallBarringFeature",
	"Ext-CallBarringFeature",
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
	asn_DEF_Ext_CallBarringFeature_1_tags,
	sizeof(asn_DEF_Ext_CallBarringFeature_1_tags)
		/sizeof(asn_DEF_Ext_CallBarringFeature_1_tags[0]), /* 1 */
	asn_DEF_Ext_CallBarringFeature_1_tags,	/* Same as above */
	sizeof(asn_DEF_Ext_CallBarringFeature_1_tags)
		/sizeof(asn_DEF_Ext_CallBarringFeature_1_tags[0]), /* 1 */
	asn_MBR_Ext_CallBarringFeature_1,
	3,	/* Elements count */
	&asn_SPC_Ext_CallBarringFeature_1_specs	/* Additional specs */
};

