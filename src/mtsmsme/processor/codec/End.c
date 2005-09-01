#include <asn_internal.h>

#include <End.h>

static asn_TYPE_member_t asn_MBR_End[] = {
  { ATF_NOFLAGS, 0, offsetof(struct End, dtid),
    (ASN_TAG_CLASS_APPLICATION | (9 << 2)),
    0,
    (void *)&asn_DEF_DestTransactionID,
    0,  /* Defer constraints checking to the member type */
    "dtid"
    },
  { ATF_POINTER, 2, offsetof(struct End, dialoguePortion),
    (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
    0,
    (void *)&asn_DEF_DialoguePortion,
    0,  /* Defer constraints checking to the member type */
    "dialoguePortion"
    },
  { ATF_POINTER, 1, offsetof(struct End, components),
    (ASN_TAG_CLASS_APPLICATION | (12 << 2)),
    0,
    (void *)&asn_DEF_ComponentPortion,
    0,  /* Defer constraints checking to the member type */
    "components"
    },
};
static ber_tlv_tag_t asn_DEF_End_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_End_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (9 << 2)), 0, 0, 0 }, /* dtid at 39 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 }, /* dialoguePortion at 40 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 2, 0, 0 }, /* components at 41 */
};
static asn_SEQUENCE_specifics_t asn_DEF_End_specs = {
  sizeof(struct End),
  offsetof(struct End, _asn_ctx),
  asn_DEF_End_tag2el,
  3,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_End = {
  "End",
  "End",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_End_tags,
  sizeof(asn_DEF_End_tags)
    /sizeof(asn_DEF_End_tags[0]), /* 1 */
  asn_DEF_End_tags, /* Same as above */
  sizeof(asn_DEF_End_tags)
    /sizeof(asn_DEF_End_tags[0]), /* 1 */
  asn_MBR_End,
  3,  /* Elements count */
  &asn_DEF_End_specs  /* Additional specs */
};
