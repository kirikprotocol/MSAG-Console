#include <asn_internal.h>

#include <Invoke.h>

static asn_TYPE_member_t asn_MBR_Invoke[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Invoke, invokeID),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_InvokeIdType,
    0,  /* Defer constraints checking to the member type */
    "invokeID"
    },
  { ATF_POINTER, 1, offsetof(struct Invoke, linkedID),
    (ASN_TAG_CLASS_CONTEXT | (0 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_InvokeIdType,
    0,  /* Defer constraints checking to the member type */
    "linkedID"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Invoke, opcode),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_Code,
    0,  /* Defer constraints checking to the member type */
    "opcode"
    },
  { ATF_OPEN_TYPE | ATF_POINTER, 1, offsetof(struct Invoke, argument),
    -1 /* Ambiguous tag (ANY?) */,
    0,
    (void *)&asn_DEF_ANY,
    0,  /* Defer constraints checking to the member type */
    "argument"
    },
};
static ber_tlv_tag_t asn_DEF_Invoke_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Invoke_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 1 }, /* invokeID at 112 */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 2, -1, 0 }, /* local at 51 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 2, 0, 0 }, /* global at 52 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* linkedID at 113 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Invoke_specs = {
  sizeof(struct Invoke),
  offsetof(struct Invoke, _asn_ctx),
  asn_DEF_Invoke_tag2el,
  4,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Invoke = {
  "Invoke",
  "Invoke",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Invoke_tags,
  sizeof(asn_DEF_Invoke_tags)
    /sizeof(asn_DEF_Invoke_tags[0]), /* 1 */
  asn_DEF_Invoke_tags,  /* Same as above */
  sizeof(asn_DEF_Invoke_tags)
    /sizeof(asn_DEF_Invoke_tags[0]), /* 1 */
  asn_MBR_Invoke,
  4,  /* Elements count */
  &asn_DEF_Invoke_specs /* Additional specs */
};
