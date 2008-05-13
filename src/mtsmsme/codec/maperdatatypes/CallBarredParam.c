#include <asn_internal.h>

#include "CallBarredParam.h"

static asn_TYPE_member_t asn_MBR_CallBarredParam_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CallBarredParam, choice.callBarringCause),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_CallBarringCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callBarringCause"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CallBarredParam, choice.extensibleCallBarredParam),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensibleCallBarredParam,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensibleCallBarredParam"
		},
};
static asn_TYPE_tag2member_t asn_MAP_CallBarredParam_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* callBarringCause at 84 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensibleCallBarredParam at 88 */
};
static asn_CHOICE_specifics_t asn_SPC_CallBarredParam_specs_1 = {
	sizeof(struct CallBarredParam),
	offsetof(struct CallBarredParam, _asn_ctx),
	offsetof(struct CallBarredParam, present),
	sizeof(((struct CallBarredParam *)0)->present),
	asn_MAP_CallBarredParam_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_CallBarredParam = {
	"CallBarredParam",
	"CallBarredParam",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_CallBarredParam_1,
	2,	/* Elements count */
	&asn_SPC_CallBarredParam_specs_1	/* Additional specs */
};

