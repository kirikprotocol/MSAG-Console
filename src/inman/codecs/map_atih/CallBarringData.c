#ident "$Id$"

#include <asn_internal.h>

#include "CallBarringData.h"

static asn_TYPE_member_t asn_MBR_CallBarringData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CallBarringData, callBarringFeatureList),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_Ext_CallBarFeatureList,
		0,	/* Defer constraints checking to the member type */
		"callBarringFeatureList"
		},
	{ ATF_POINTER, 4, offsetof(struct CallBarringData, password),
		(ASN_TAG_CLASS_UNIVERSAL | (18 << 2)),
		0,
		&asn_DEF_Password,
		0,	/* Defer constraints checking to the member type */
		"password"
		},
	{ ATF_POINTER, 3, offsetof(struct CallBarringData, wrongPasswordAttemptsCounter),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_WrongPasswordAttemptsCounter,
		0,	/* Defer constraints checking to the member type */
		"wrongPasswordAttemptsCounter"
		},
	{ ATF_POINTER, 2, offsetof(struct CallBarringData, notificationToCSE),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct CallBarringData, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_CallBarringData_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_CallBarringData_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 2, 0, 0 }, /* wrongPasswordAttemptsCounter at 819 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 3, 0, 0 }, /* notificationToCSE at 820 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 1 }, /* callBarringFeatureList at 817 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 4, -1, 0 }, /* extensionContainer at 821 */
    { (ASN_TAG_CLASS_UNIVERSAL | (18 << 2)), 1, 0, 0 } /* password at 818 */
};
static asn_SEQUENCE_specifics_t asn_SPC_CallBarringData_1_specs = {
	sizeof(struct CallBarringData),
	offsetof(struct CallBarringData, _asn_ctx),
	asn_MAP_CallBarringData_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_CallBarringData = {
	"CallBarringData",
	"CallBarringData",
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
	asn_DEF_CallBarringData_1_tags,
	sizeof(asn_DEF_CallBarringData_1_tags)
		/sizeof(asn_DEF_CallBarringData_1_tags[0]), /* 1 */
	asn_DEF_CallBarringData_1_tags,	/* Same as above */
	sizeof(asn_DEF_CallBarringData_1_tags)
		/sizeof(asn_DEF_CallBarringData_1_tags[0]), /* 1 */
	asn_MBR_CallBarringData_1,
	5,	/* Elements count */
	&asn_SPC_CallBarringData_1_specs	/* Additional specs */
};

