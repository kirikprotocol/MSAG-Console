#ident "$Id$"

#include <asn_internal.h>

#include "M-CSI.h"

static asn_TYPE_member_t asn_MBR_M_CSI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct M_CSI, mobilityTriggers),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_MobilityTriggers,
		0,	/* Defer constraints checking to the member type */
		"mobilityTriggers"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct M_CSI, serviceKey),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct M_CSI, gsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_POINTER, 3, offsetof(struct M_CSI, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct M_CSI, notificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct M_CSI, csi_Active),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"csi-Active"
		},
};
static ber_tlv_tag_t asn_DEF_M_CSI_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_M_CSI_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 }, /* serviceKey at 669 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 0 }, /* mobilityTriggers at 668 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 }, /* gsmSCF-Address at 670 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* extensionContainer at 671 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 4, 0, 0 }, /* notificationToCSE at 672 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 5, 0, 0 } /* csi-Active at 673 */
};
static asn_SEQUENCE_specifics_t asn_SPC_M_CSI_1_specs = {
	sizeof(struct M_CSI),
	offsetof(struct M_CSI, _asn_ctx),
	asn_MAP_M_CSI_1_tag2el,
	6,	/* Count of tags in the map */
	5,	/* Start extensions */
	7	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_M_CSI = {
	"M-CSI",
	"M-CSI",
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
	asn_DEF_M_CSI_1_tags,
	sizeof(asn_DEF_M_CSI_1_tags)
		/sizeof(asn_DEF_M_CSI_1_tags[0]), /* 1 */
	asn_DEF_M_CSI_1_tags,	/* Same as above */
	sizeof(asn_DEF_M_CSI_1_tags)
		/sizeof(asn_DEF_M_CSI_1_tags[0]), /* 1 */
	asn_MBR_M_CSI_1,
	6,	/* Elements count */
	&asn_SPC_M_CSI_1_specs	/* Additional specs */
};

