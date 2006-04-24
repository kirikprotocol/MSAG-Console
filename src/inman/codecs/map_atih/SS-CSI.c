#ident "$Id$"

#include <asn_internal.h>

#include "SS-CSI.h"

static asn_TYPE_member_t asn_MBR_SS_CSI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SS_CSI, ss_CamelData),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_SS_CamelData,
		0,	/* Defer constraints checking to the member type */
		"ss-CamelData"
		},
	{ ATF_POINTER, 3, offsetof(struct SS_CSI, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct SS_CSI, notificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct SS_CSI, csi_Active),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"csi-Active"
		},
};
static ber_tlv_tag_t asn_DEF_SS_CSI_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SS_CSI_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 1 }, /* ss-CamelData at 443 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, -1, 0 }, /* extensionContainer at 444 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 }, /* notificationToCSE at 446 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 } /* csi-Active at 447 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SS_CSI_1_specs = {
	sizeof(struct SS_CSI),
	offsetof(struct SS_CSI, _asn_ctx),
	asn_MAP_SS_CSI_1_tag2el,
	4,	/* Count of tags in the map */
	1,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SS_CSI = {
	"SS-CSI",
	"SS-CSI",
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
	asn_DEF_SS_CSI_1_tags,
	sizeof(asn_DEF_SS_CSI_1_tags)
		/sizeof(asn_DEF_SS_CSI_1_tags[0]), /* 1 */
	asn_DEF_SS_CSI_1_tags,	/* Same as above */
	sizeof(asn_DEF_SS_CSI_1_tags)
		/sizeof(asn_DEF_SS_CSI_1_tags[0]), /* 1 */
	asn_MBR_SS_CSI_1,
	4,	/* Elements count */
	&asn_SPC_SS_CSI_1_specs	/* Additional specs */
};

