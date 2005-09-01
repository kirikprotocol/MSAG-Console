#include <asn_internal.h>

#include <Reject.h>

static asn_TYPE_member_t asn_MBR_invokeID[] = {
  { ATF_NOFLAGS, 0, offsetof(struct invokeID, choice.derivable),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_InvokeIdType,
    0,  /* Defer constraints checking to the member type */
    "derivable"
    },
  { ATF_NOFLAGS, 0, offsetof(struct invokeID, choice.not_derivable),
    (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
    0,
    (void *)&asn_DEF_NULL,
    0,  /* Defer constraints checking to the member type */
    "not-derivable"
    },
};
static asn_TYPE_tag2member_t asn_DEF_invokeID_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* derivable at 137 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 1, 0, 0 }, /* not-derivable at 138 */
};
static asn_CHOICE_specifics_t asn_DEF_invokeID_specs = {
  sizeof(struct invokeID),
  offsetof(struct invokeID, _asn_ctx),
  offsetof(struct invokeID, present),
  sizeof(((struct invokeID *)0)->present),
  asn_DEF_invokeID_tag2el,
  2,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_invokeID = {
  "invokeID",
  "invokeID",
  CHOICE_free,
  CHOICE_print,
  CHOICE_constraint,
  CHOICE_decode_ber,
  CHOICE_encode_der,
  CHOICE_decode_xer,
  CHOICE_encode_xer,
  CHOICE_outmost_tag,
  0,  /* No effective tags (pointer) */
  0,  /* No effective tags (count) */
  0,  /* No tags (pointer) */
  0,  /* No tags (count) */
  asn_MBR_invokeID,
  2,  /* Elements count */
  &asn_DEF_invokeID_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_problem[] = {
  { ATF_NOFLAGS, 0, offsetof(struct problem, choice.generalProblem),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_GeneralProblem,
    0,  /* Defer constraints checking to the member type */
    "generalProblem"
    },
  { ATF_NOFLAGS, 0, offsetof(struct problem, choice.invokeProblem),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_InvokeProblem,
    0,  /* Defer constraints checking to the member type */
    "invokeProblem"
    },
  { ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnResultProblem),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_ReturnResultProblem,
    0,  /* Defer constraints checking to the member type */
    "returnResultProblem"
    },
  { ATF_NOFLAGS, 0, offsetof(struct problem, choice.returnErrorProblem),
    (ASN_TAG_CLASS_CONTEXT | (3 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_ReturnErrorProblem,
    0,  /* Defer constraints checking to the member type */
    "returnErrorProblem"
    },
};
static asn_TYPE_tag2member_t asn_DEF_problem_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* generalProblem at 142 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invokeProblem at 143 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* returnResultProblem at 144 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* returnErrorProblem at 146 */
};
static asn_CHOICE_specifics_t asn_DEF_problem_specs = {
  sizeof(struct problem),
  offsetof(struct problem, _asn_ctx),
  offsetof(struct problem, present),
  sizeof(((struct problem *)0)->present),
  asn_DEF_problem_tag2el,
  4,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_problem = {
  "problem",
  "problem",
  CHOICE_free,
  CHOICE_print,
  CHOICE_constraint,
  CHOICE_decode_ber,
  CHOICE_encode_der,
  CHOICE_decode_xer,
  CHOICE_encode_xer,
  CHOICE_outmost_tag,
  0,  /* No effective tags (pointer) */
  0,  /* No effective tags (count) */
  0,  /* No tags (pointer) */
  0,  /* No tags (count) */
  asn_MBR_problem,
  4,  /* Elements count */
  &asn_DEF_problem_specs  /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_Reject[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Reject, invokeID),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_invokeID,
    0,  /* Defer constraints checking to the member type */
    "invokeID"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Reject, problem),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_problem,
    0,  /* Defer constraints checking to the member type */
    "problem"
    },
};
static ber_tlv_tag_t asn_DEF_Reject_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Reject_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* derivable at 137 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 0, 0, 0 }, /* not-derivable at 138 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* generalProblem at 142 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* invokeProblem at 143 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* returnResultProblem at 144 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 }, /* returnErrorProblem at 146 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Reject_specs = {
  sizeof(struct Reject),
  offsetof(struct Reject, _asn_ctx),
  asn_DEF_Reject_tag2el,
  6,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
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
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Reject_tags,
  sizeof(asn_DEF_Reject_tags)
    /sizeof(asn_DEF_Reject_tags[0]), /* 1 */
  asn_DEF_Reject_tags,  /* Same as above */
  sizeof(asn_DEF_Reject_tags)
    /sizeof(asn_DEF_Reject_tags[0]), /* 1 */
  asn_MBR_Reject,
  2,  /* Elements count */
  &asn_DEF_Reject_specs /* Additional specs */
};
