#include <asn_internal.h>

#include <DialoguePDU.h>

static asn_TYPE_member_t asn_MBR_DialoguePDU[] = {
  { ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueRequest),
    (ASN_TAG_CLASS_APPLICATION | (0 << 2)),
    0,
    (void *)&asn_DEF_AARQ_apdu,
    0,  /* Defer constraints checking to the member type */
    "dialogueRequest"
    },
  { ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueResponse),
    (ASN_TAG_CLASS_APPLICATION | (1 << 2)),
    0,
    (void *)&asn_DEF_AARE_apdu,
    0,  /* Defer constraints checking to the member type */
    "dialogueResponse"
    },
  { ATF_NOFLAGS, 0, offsetof(struct DialoguePDU, choice.dialogueAbort),
    (ASN_TAG_CLASS_APPLICATION | (4 << 2)),
    0,
    (void *)&asn_DEF_ABRT_apdu,
    0,  /* Defer constraints checking to the member type */
    "dialogueAbort"
    },
};
static asn_TYPE_tag2member_t asn_DEF_DialoguePDU_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (0 << 2)), 0, 0, 0 }, /* dialogueRequest at 13 */
    { (ASN_TAG_CLASS_APPLICATION | (1 << 2)), 1, 0, 0 }, /* dialogueResponse at 14 */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 2, 0, 0 }, /* dialogueAbort at 16 */
};
static asn_CHOICE_specifics_t asn_DEF_DialoguePDU_specs = {
  sizeof(struct DialoguePDU),
  offsetof(struct DialoguePDU, _asn_ctx),
  offsetof(struct DialoguePDU, present),
  sizeof(((struct DialoguePDU *)0)->present),
  asn_DEF_DialoguePDU_tag2el,
  3,  /* Count of tags in the map */
  0 /* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_DialoguePDU = {
  "DialoguePDU",
  "DialoguePDU",
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
  asn_MBR_DialoguePDU,
  3,  /* Elements count */
  &asn_DEF_DialoguePDU_specs  /* Additional specs */
};
