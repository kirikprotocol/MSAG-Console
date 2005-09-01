#include <asn_internal.h>

#include <Error.h>

static asn_TYPE_member_t asn_MBR_Error[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Error, choice.local),
    (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
    0,
    (void *)&asn_DEF_NativeInteger,
    0,  /* Defer constraints checking to the member type */
    "local"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Error, choice.global),
    (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)),
    0,
    (void *)&asn_DEF_OBJECT_IDENTIFIER,
    0,  /* Defer constraints checking to the member type */
    "global"
    },
};
static asn_TYPE_tag2member_t asn_DEF_Error_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* local at 54 */
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 1, 0, 0 }, /* global at 55 */
};
static asn_CHOICE_specifics_t asn_DEF_Error_specs = {
  sizeof(struct Error),
  offsetof(struct Error, _asn_ctx),
  offsetof(struct Error, present),
  sizeof(((struct Error *)0)->present),
  asn_DEF_Error_tag2el,
  2,  /* Count of tags in the map */
  0 /* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_Error = {
  "Error",
  "Error",
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
  asn_MBR_Error,
  2,  /* Elements count */
  &asn_DEF_Error_specs  /* Additional specs */
};
