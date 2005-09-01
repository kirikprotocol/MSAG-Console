#include <asn_internal.h>

#include <ReturnError.h>

static asn_TYPE_member_t asn_MBR_ReturnError[] = {
  { ATF_NOFLAGS, 0, offsetof(struct ReturnError, invokeID),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_InvokeIdType,
    0,  /* Defer constraints checking to the member type */
    "invokeID"
    },
  { ATF_NOFLAGS, 0, offsetof(struct ReturnError, errorCode),
    -1 /* Ambiguous tag (CHOICE?) */,
    0,
    (void *)&asn_DEF_Error,
    0,  /* Defer constraints checking to the member type */
    "errorCode"
    },
  { ATF_OPEN_TYPE | ATF_POINTER, 1, offsetof(struct ReturnError, parameter),
    -1 /* Ambiguous tag (ANY?) */,
    0,
    (void *)&asn_DEF_ANY,
    0,  /* Defer constraints checking to the member type */
    "parameter"
    },
};
static ber_tlv_tag_t asn_DEF_ReturnError_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_ReturnError_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 1 }, /* invokeID at 128 */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, -1, 0 }, /* local at 54 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 1, 0, 0 }, /* global at 55 */
};
static asn_SEQUENCE_specifics_t asn_DEF_ReturnError_specs = {
  sizeof(struct ReturnError),
  offsetof(struct ReturnError, _asn_ctx),
  asn_DEF_ReturnError_tag2el,
  3,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ReturnError = {
  "ReturnError",
  "ReturnError",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_ReturnError_tags,
  sizeof(asn_DEF_ReturnError_tags)
    /sizeof(asn_DEF_ReturnError_tags[0]), /* 1 */
  asn_DEF_ReturnError_tags, /* Same as above */
  sizeof(asn_DEF_ReturnError_tags)
    /sizeof(asn_DEF_ReturnError_tags[0]), /* 1 */
  asn_MBR_ReturnError,
  3,  /* Elements count */
  &asn_DEF_ReturnError_specs  /* Additional specs */
};
