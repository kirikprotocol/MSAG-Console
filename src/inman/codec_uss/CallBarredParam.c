#include <asn_internal.h>

#include "CallBarredParam.h"

static asn_TYPE_member_t asn_MBR_CallBarredParam_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CallBarredParam, choice.callBarringCause),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CallBarringCause,
		0,	/* Defer constraints checking to the member type */
		"callBarringCause"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CallBarredParam, choice.extensibleCallBarredParam),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensibleCallBarredParam,
		0,	/* Defer constraints checking to the member type */
		"extensibleCallBarredParam"
		},
};
static asn_TYPE_tag2member_t asn_MAP_CallBarredParam_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* callBarringCause at 294 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensibleCallBarredParam at 298 */
};
static asn_CHOICE_specifics_t asn_SPC_CallBarredParam_1_specs = {
	sizeof(struct CallBarredParam),
	offsetof(struct CallBarredParam, _asn_ctx),
	offsetof(struct CallBarredParam, present),
	sizeof(((struct CallBarredParam *)0)->present),
	asn_MAP_CallBarredParam_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_CallBarredParam = {
	"CallBarredParam",
	"CallBarredParam",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
#ifndef ASN1_XER_NOT_USED
	CHOICE_decode_xer,
	CHOICE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_CallBarredParam_1,
	2,	/* Elements count */
	&asn_SPC_CallBarredParam_1_specs	/* Additional specs */
};

