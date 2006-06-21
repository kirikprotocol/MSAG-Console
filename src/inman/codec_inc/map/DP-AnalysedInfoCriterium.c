#ident "$Id$"

#include <asn_internal.h>

#include "DP-AnalysedInfoCriterium.h"

static asn_TYPE_member_t asn_MBR_DP_AnalysedInfoCriterium_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, dialledNumber),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"dialledNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, serviceKey),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, gsmSCF_Address),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, defaultCallHandling),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_DefaultCallHandling,
		0,	/* Defer constraints checking to the member type */
		"defaultCallHandling"
		},
	{ ATF_POINTER, 1, offsetof(struct DP_AnalysedInfoCriterium, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_DP_AnalysedInfoCriterium_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DP_AnalysedInfoCriterium_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 }, /* serviceKey at 172 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 1 }, /* dialledNumber at 171 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, -1, 0 }, /* gsmSCF-Address at 173 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 3, 0, 0 }, /* defaultCallHandling at 174 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 4, 0, 0 } /* extensionContainer at 175 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DP_AnalysedInfoCriterium_1_specs = {
	sizeof(struct DP_AnalysedInfoCriterium),
	offsetof(struct DP_AnalysedInfoCriterium, _asn_ctx),
	asn_MAP_DP_AnalysedInfoCriterium_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DP_AnalysedInfoCriterium = {
	"DP-AnalysedInfoCriterium",
	"DP-AnalysedInfoCriterium",
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
	asn_DEF_DP_AnalysedInfoCriterium_1_tags,
	sizeof(asn_DEF_DP_AnalysedInfoCriterium_1_tags)
		/sizeof(asn_DEF_DP_AnalysedInfoCriterium_1_tags[0]), /* 1 */
	asn_DEF_DP_AnalysedInfoCriterium_1_tags,	/* Same as above */
	sizeof(asn_DEF_DP_AnalysedInfoCriterium_1_tags)
		/sizeof(asn_DEF_DP_AnalysedInfoCriterium_1_tags[0]), /* 1 */
	asn_MBR_DP_AnalysedInfoCriterium_1,
	5,	/* Elements count */
	&asn_SPC_DP_AnalysedInfoCriterium_1_specs	/* Additional specs */
};

