#include "sms_tags.h"

namespace smsc{
namespace sms{
namespace Tag{

int tagTypes[SMS_LAST_TAG+1];

static struct TagTypesInitializer{
TagTypesInitializer()
{
#undef __SMSC_SMS_SMS_TAGS_H__
#undef SMSDEFTAG
#define SMSDEFTAG(type,idx,name) tagTypes[idx]=type;
#define _TAGS_INTERNAL_
#include "sms_tags.h"
SMS_BODY_TAGS_SET
}
}dummy;

};
};
};
