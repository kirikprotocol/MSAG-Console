#include <asn_internal.h>

#include "O-BcsmCamelTDPData.h"

static asn_TYPE_member_t asn_MBR_O_BcsmCamelTDPData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct O_BcsmCamelTDPData, o_BcsmTriggerDetectionPoint),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_O_BcsmTriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"o-BcsmTriggerDetectionPoint"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct O_BcsmCamelTDPData, serviceKey),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct O_BcsmCamelTDPData, gsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsmSCF-Address"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct O_BcsmCamelTDPData, defaultCallHandling),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DefaultCallHandling,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"defaultCallHandling"
		},
	{ ATF_POINTER, 1, offsetof(struct O_BcsmCamelTDPData, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_O_BcsmCamelTDPData_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_O_BcsmCamelTDPData_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 }, /* serviceKey at 211 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* o-BcsmTriggerDetectionPoint at 210 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 }, /* gsmSCF-Address at 212 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* defaultCallHandling at 213 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 4, 0, 0 } /* extensionContainer at 214 */
};
static asn_SEQUENCE_specifics_t asn_SPC_O_BcsmCamelTDPData_specs_1 = {
	sizeof(struct O_BcsmCamelTDPData),
	offsetof(struct O_BcsmCamelTDPData, _asn_ctx),
	asn_MAP_O_BcsmCamelTDPData_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_O_BcsmCamelTDPData = {
	"O-BcsmCamelTDPData",
	"O-BcsmCamelTDPData",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_O_BcsmCamelTDPData_tags_1,
	sizeof(asn_DEF_O_BcsmCamelTDPData_tags_1)
		/sizeof(asn_DEF_O_BcsmCamelTDPData_tags_1[0]), /* 1 */
	asn_DEF_O_BcsmCamelTDPData_tags_1,	/* Same as above */
	sizeof(asn_DEF_O_BcsmCamelTDPData_tags_1)
		/sizeof(asn_DEF_O_BcsmCamelTDPData_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_O_BcsmCamelTDPData_1,
	5,	/* Elements count */
	&asn_SPC_O_BcsmCamelTDPData_specs_1	/* Additional specs */
};

