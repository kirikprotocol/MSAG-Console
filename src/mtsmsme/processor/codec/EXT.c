#include <asn_internal.h>

#include <EXT.h>

static asn_TYPE_member_t asn_MBR_encoding[] = {
  { ATF_NOFLAGS, 0, offsetof(struct encoding, choice.single_ASN1_type),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    +1, /* EXPLICIT tag at current level */
    (void *)&asn_DEF_DialoguePDU,
    0,  /* Defer constraints checking to the member type */
    "single-ASN1-type"
    },
  { ATF_NOFLAGS, 0, offsetof(struct encoding, choice.octet_aligned),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_OCTET_STRING,
    0,  /* Defer constraints checking to the member type */
    "octet-aligned"
    },
  { ATF_NOFLAGS, 0, offsetof(struct encoding, choice.arbitrary),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_BIT_STRING,
    0,  /* Defer constraints checking to the member type */
    "arbitrary"
    },
};
static asn_TYPE_tag2member_t asn_DEF_encoding_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* single-ASN1-type at 68 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* octet-aligned at 69 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* arbitrary at 70 */
};
static asn_CHOICE_specifics_t asn_DEF_encoding_specs = {
  sizeof(struct encoding),
  offsetof(struct encoding, _asn_ctx),
  offsetof(struct encoding, present),
  sizeof(((struct encoding *)0)->present),
  asn_DEF_encoding_tag2el,
  3,  /* Count of tags in the map */
  0 /* Whether extensible */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_encoding = {
  "encoding",
  "encoding",
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
  asn_MBR_encoding,
  3,  /* Elements count */
  &asn_DEF_encoding_specs /* Additional specs */
};

static asn_TYPE_member_t asn_MBR_EXT[] = {
  { ATF_POINTER, 3, offsetof(struct EXT, direct_reference),
    (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)),
    0,
    (void *)&asn_DEF_OBJECT_IDENTIFIER,
    0,  /* Defer constraints checking to the member type */
    "direct-reference"
    },
  { ATF_POINTER, 2, offsetof(struct EXT, indirect_reference),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_NativeInteger,
    0,  /* Defer constraints checking to the member type */
    "indirect-reference"
    },
  { ATF_POINTER, 1, offsetof(struct EXT, data_value_descriptor),
    (ASN_TAG_CLASS_UNIVERSAL | (7 << 2)),
    0,
    (void *)&asn_DEF_ObjectDescriptor,
    0,  /* Defer constraints checking to the member type */
    "data-value-descriptor"
    },
  { ATF_NOFLAGS, 0, offsetof(struct EXT, encoding),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_encoding,
    0,  /* Defer constraints checking to the member type */
    "encoding"
    },
};
static ber_tlv_tag_t asn_DEF_EXT_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (8 << 2)),
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_EXT_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, 0, 0 }, /* indirect-reference at 65 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 }, /* direct-reference at 64 */
    { (ASN_TAG_CLASS_UNIVERSAL | (7 << 2)), 2, 0, 0 }, /* data-value-descriptor at 66 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 3, 0, 0 }, /* single-ASN1-type at 68 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* octet-aligned at 69 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 }, /* arbitrary at 70 */
};
static asn_SEQUENCE_specifics_t asn_DEF_EXT_specs = {
  sizeof(struct EXT),
  offsetof(struct EXT, _asn_ctx),
  asn_DEF_EXT_tag2el,
  6,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_EXT = {
  "EXT",
  "EXT",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_EXT_tags,
  sizeof(asn_DEF_EXT_tags)
    /sizeof(asn_DEF_EXT_tags[0]) - 1, /* 1 */
  asn_DEF_EXT_tags, /* Same as above */
  sizeof(asn_DEF_EXT_tags)
    /sizeof(asn_DEF_EXT_tags[0]), /* 2 */
  asn_MBR_EXT,
  4,  /* Elements count */
  &asn_DEF_EXT_specs  /* Additional specs */
};
