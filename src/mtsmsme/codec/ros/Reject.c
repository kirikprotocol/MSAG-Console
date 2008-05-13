#include <asn_internal.h>

#include "Reject.h"

static asn_TYPE_member_t asn_MBR_problem_3[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.general),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeneralProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"general"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.invoke),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_InvokeProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invoke"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnResult),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnResultProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnResult"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnError),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnErrorProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnError"
		},
};
static asn_TYPE_tag2member_t asn_MAP_problem_tag2el_3[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* general at 106 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invoke at 107 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* returnResult at 108 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* returnError at 109 */
};
static asn_CHOICE_specifics_t asn_SPC_problem_specs_3 = {
	sizeof(struct problem),
	offsetof(struct problem, _asn_ctx),
	offsetof(struct problem, present),
	sizeof(((struct problem *)0)->present),
	asn_MAP_problem_tag2el_3,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_problem_3 = {
	"problem",
	"problem",
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
	asn_MBR_problem_3,
	4,	/* Elements count */
	&asn_SPC_problem_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_Reject_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Reject, invokeId),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_InvokeId,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invokeId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Reject, problem),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_problem_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"problem"
		},
};
static ber_tlv_tag_t asn_DEF_Reject_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Reject_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* invokeId at 104 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* general at 106 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invoke at 107 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* returnResult at 108 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 } /* returnError at 109 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Reject_specs_1 = {
	sizeof(struct Reject),
	offsetof(struct Reject, _asn_ctx),
	asn_MAP_Reject_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Reject = {
	"Reject",
	"Reject",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Reject_tags_1,
	sizeof(asn_DEF_Reject_tags_1)
		/sizeof(asn_DEF_Reject_tags_1[0]), /* 1 */
	asn_DEF_Reject_tags_1,	/* Same as above */
	sizeof(asn_DEF_Reject_tags_1)
		/sizeof(asn_DEF_Reject_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Reject_1,
	2,	/* Elements count */
	&asn_SPC_Reject_specs_1	/* Additional specs */
};

