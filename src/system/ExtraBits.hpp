#ifndef __SMSC_SYSTEM_EXTRABITS_HPP__
#define __SMSC_SYSTEM_EXTRABITS_HPP__

#include "util/int.h"

namespace smsc{
namespace system{

static const uint32_t EXTRA_NICK  =0x0001;
static const uint32_t EXTRA_FLASH =0x0002;
static const uint32_t EXTRA_CALEND=0x0004;
static const uint32_t EXTRA_SECRET=0x0008;

}//system
}//smsc

#endif
