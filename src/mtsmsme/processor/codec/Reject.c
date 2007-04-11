#include <asn_internal.h>

#include "Reject.h"

static asn_TYPE_member_t asn_MBR_invokeID_2[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct invokeID, choice.derivable),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_InvokeIdType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"derivable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct invokeID, choice.not_derivable),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"not-derivable"
		},
};
static asn_TYPE_tag2member_t asn_MAP_invokeID_tag2el_2[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* derivable at 138 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 } /* not-derivable at 139 */
};
static asn_CHOICE_specifics_t asn_SPC_invokeID_specs_2 = {
	sizeof(struct invokeID),
	offsetof(struct invokeID, _asn_ctx),
	offsetof(struct invokeID, present),
	sizeof(((struct invokeID *)0)->present),
	asn_MAP_invokeID_tag2el_2,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_invokeID_2 = {
	"invokeID",
	"invokeID",
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
	asn_MBR_invokeID_2,
	2,	/* Elements count */
	&asn_SPC_invokeID_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_problem_5[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.generalProblem),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeneralProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"generalProblem"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.invokeProblem),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_InvokeProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invokeProblem"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnResultProblem),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnResultProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnResultProblem"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnErrorProblem),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnErrorProblem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnErrorProblem"
		},
};
static asn_TYPE_tag2member_t asn_MAP_problem_tag2el_5[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* generalProblem at 143 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invokeProblem at 144 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* returnResultProblem at 145 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* returnErrorProblem at 147 */
};
static asn_CHOICE_specifics_t asn_SPC_problem_specs_5 = {
	sizeof(struct problem),
	offsetof(struct problem, _asn_ctx),
	offsetof(struct problem, present),
	sizeof(((struct problem *)0)->present),
	asn_MAP_problem_tag2el_5,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_problem_5 = {
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
	asn_MBR_problem_5,
	4,	/* Elements count */
	&asn_SPC_problem_specs_5	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_Reject_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Reject, invokeID),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_invokeID_2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invokeID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Reject, problem),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_problem_5,
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
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* derivable at 138 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 0, 0, 0 }, /* not-derivable at 139 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* generalProblem at 143 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invokeProblem at 144 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* returnResultProblem at 145 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 } /* returnErrorProblem at 147 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Reject_specs_1 = {
	sizeof(struct Reject),
	offsetof(struct Reject, _asn_ctx),
	asn_MAP_Reject_tag2el_1,
	6,	/* Count of tags in the map */
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

