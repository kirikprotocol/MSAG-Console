#include <asn_internal.h>

#include <Abort.h>

static asn_TYPE_member_t asn_MBR_reason[] = {
  { ATF_NOFLAGS, 0, offsetof(struct reason, choice.p_abortCause),
    (ASN_TAG_CLASS_APPLICATION | (10 << 2)),
    0,
    (void *)&asn_DEF_P_AbortCause,
    0,  /* Defer constraints checking to the member type */
    "p-abortCause"
    },
  { ATF_NOFLAGS, 0, offsetof(struct reason, choice.dialoguePortion),
    (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
    0,
    (void *)&asn_DEF_DialoguePortion,
    0,  /* Defer constraints checking to the member type */
    "dialoguePortion"
    },
};
static asn_TYPE_tag2member_t asn_DEF_reason_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (10 << 2)), 0, 0, 0 }, /* p-abortCause at 55 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 }, /* dialoguePortion at 57 */
};
static asn_CHOICE_specifics_t asn_DEF_reason_specs = {
  sizeof(struct reason),
  offsetof(struct reason, _asn_ctx),
  offsetof(struct reason, present),
  sizeof(((struct reason *)0)->present),
  asn_DEF_reason_tag2el,
  2,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_reason = {
  "reason",
  "reason",
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
  asn_MBR_reason,
  2,  /* Elements count */
  &asn_DEF_reason_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_Abort[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Abort, dtid),
    (ASN_TAG_CLASS_APPLICATION | (9 << 2)),
    0,
    (void *)&asn_DEF_DestTransactionID,
    0,  /* Defer constraints checking to the member type */
    "dtid"
    },
  { ATF_POINTER, 1, offsetof(struct Abort, reason),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_reason,
    0,  /* Defer constraints checking to the member type */
    "reason"
    },
};
static ber_tlv_tag_t asn_DEF_Abort_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Abort_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (9 << 2)), 0, 0, 0 }, /* dtid at 52 */
    { (ASN_TAG_CLASS_APPLICATION | (10 << 2)), 1, 0, 0 }, /* p-abortCause at 55 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 }, /* dialoguePortion at 57 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Abort_specs = {
  sizeof(struct Abort),
  offsetof(struct Abort, _asn_ctx),
  asn_DEF_Abort_tag2el,
  3,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Abort = {
  "Abort",
  "Abort",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Abort_tags,
  sizeof(asn_DEF_Abort_tags)
    /sizeof(asn_DEF_Abort_tags[0]), /* 1 */
  asn_DEF_Abort_tags, /* Same as above */
  sizeof(asn_DEF_Abort_tags)
    /sizeof(asn_DEF_Abort_tags[0]), /* 1 */
  asn_MBR_Abort,
  2,  /* Elements count */
  &asn_DEF_Abort_specs  /* Additional specs */
};
