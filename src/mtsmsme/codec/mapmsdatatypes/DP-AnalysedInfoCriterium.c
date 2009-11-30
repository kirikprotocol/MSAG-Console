#include <asn_internal.h>

#include "DP-AnalysedInfoCriterium.h"

static asn_TYPE_member_t asn_MBR_DP_AnalysedInfoCriterium_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, dialledNumber),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialledNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, serviceKey),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, gsmSCF_Address),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsmSCF-Address"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DP_AnalysedInfoCriterium, defaultCallHandling),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_DefaultCallHandling,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"defaultCallHandling"
		},
	{ ATF_POINTER, 1, offsetof(struct DP_AnalysedInfoCriterium, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_DP_AnalysedInfoCriterium_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DP_AnalysedInfoCriterium_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 }, /* serviceKey at 183 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 1 }, /* dialledNumber at 182 */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, -1, 0 }, /* gsmSCF-Address at 184 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 3, 0, 0 }, /* defaultCallHandling at 185 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 4, 0, 0 } /* extensionContainer at 186 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DP_AnalysedInfoCriterium_specs_1 = {
	sizeof(struct DP_AnalysedInfoCriterium),
	offsetof(struct DP_AnalysedInfoCriterium, _asn_ctx),
	asn_MAP_DP_AnalysedInfoCriterium_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
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
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DP_AnalysedInfoCriterium_tags_1,
	sizeof(asn_DEF_DP_AnalysedInfoCriterium_tags_1)
		/sizeof(asn_DEF_DP_AnalysedInfoCriterium_tags_1[0]), /* 1 */
	asn_DEF_DP_AnalysedInfoCriterium_tags_1,	/* Same as above */
	sizeof(asn_DEF_DP_AnalysedInfoCriterium_tags_1)
		/sizeof(asn_DEF_DP_AnalysedInfoCriterium_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DP_AnalysedInfoCriterium_1,
	5,	/* Elements count */
	&asn_SPC_DP_AnalysedInfoCriterium_specs_1	/* Additional specs */
};

