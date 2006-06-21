#ident "$Id$"

#include <asn_internal.h>

#include "ExternalSignalInfo.h"

static asn_TYPE_member_t asn_MBR_ExternalSignalInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ExternalSignalInfo, protocolId),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_ProtocolId,
		0,	/* Defer constraints checking to the member type */
		"protocolId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ExternalSignalInfo, signalInfo),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_SignalInfo,
		0,	/* Defer constraints checking to the member type */
		"signalInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct ExternalSignalInfo, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_ExternalSignalInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ExternalSignalInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* signalInfo at 177 */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* protocolId at 176 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 } /* extensionContainer at 180 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ExternalSignalInfo_1_specs = {
	sizeof(struct ExternalSignalInfo),
	offsetof(struct ExternalSignalInfo, _asn_ctx),
	asn_MAP_ExternalSignalInfo_1_tag2el,
	3,	/* Count of tags in the map */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ExternalSignalInfo = {
	"ExternalSignalInfo",
	"ExternalSignalInfo",
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
	asn_DEF_ExternalSignalInfo_1_tags,
	sizeof(asn_DEF_ExternalSignalInfo_1_tags)
		/sizeof(asn_DEF_ExternalSignalInfo_1_tags[0]), /* 1 */
	asn_DEF_ExternalSignalInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_ExternalSignalInfo_1_tags)
		/sizeof(asn_DEF_ExternalSignalInfo_1_tags[0]), /* 1 */
	asn_MBR_ExternalSignalInfo_1,
	3,	/* Elements count */
	&asn_SPC_ExternalSignalInfo_1_specs	/* Additional specs */
};

