// 
// File:   IconvRegistry.cpp
// Author: igork
//
// Created on 1 Март 2004 г., 15:51
//

#include "IconvRegistry.h"

#include <stdlib.h>
#include <map>
#include <utility>
#include <functional>

#include "core/synchronization/Mutex.hpp"
#include "util/cstrings.h"
#include "logger/Logger.h"

namespace smsc {
namespace util {
namespace xml {

using namespace smsc::core::synchronization;
using smsc::logger::Logger;

typedef std::pair<const char *, const char *> _IconvInfo;

bool operator==(const _IconvInfo& ici1, const _IconvInfo& ici2)
{
  return cStringEquals(ici1.first, ici2.first) && cStringEquals(ici1.second, ici2.second);
}

bool operator<(const _IconvInfo& ici1, const _IconvInfo& ici2)
{
  return   cStringLess(ici1.first,ici2.first) 
        || (cStringEquals(ici1.first, ici2.first) && cStringLess(ici1.second, ici2.second));
}



struct _IconvLess : public std::binary_function <_IconvInfo, _IconvInfo, bool> 
{
  bool operator()(const _IconvInfo& _Left, const _IconvInfo& _Right) const
  {
    return _Left < _Right;
  }
};

typedef std::map<_IconvInfo, iconv_t, _IconvLess> _IconvReg;
static _IconvReg iconv_reg;
Mutex globel_mutex;
#ifdef SMSC_DEBUG
static long iconv_instanceCounter = 0;
#endif //SMSC_DEBUG



/************************* Iconv Registry ****************************/

iconv_t getIconv(const char * const from, const char * const to) throw ()
{
  MutexGuard guard(globel_mutex);
  _IconvInfo info(from, to);
  if (iconv_reg.find(info) == iconv_reg.end())
  {
    #ifdef SMSC_DEBUG
    smsc_log_debug(Logger::getInstance("u.x.Iconv"), "try to get new iconv #%ld [\"%s\" -> \"%s\"]", iconv_instanceCounter, from, to);
    #endif //SMSC_DEBUG
    iconv_t ih = iconv_open(from, to);
    if (ih == (iconv_t) -1)
      return ih;
    _IconvInfo info_copy(cStringCopy(from), cStringCopy(to));
    iconv_reg[info_copy] = ih;
    #ifdef SMSC_DEBUG
    smsc_log_debug(Logger::getInstance("u.x.Iconv"), "got new iconv #%ld", iconv_instanceCounter++);
    #endif //SMSC_DEBUG
    return ih;
  } else {
    return iconv_reg[info];
  }
}

void clearIconvs() throw()
{
  MutexGuard guard(globel_mutex);
  #ifdef SMSC_DEBUG
  smsc_log_debug(Logger::getInstance("u.x.Iconv"), "clear iconv registry: %ld", iconv_instanceCounter);
  #endif //SMSC_DEBUG
  for (_IconvReg::const_iterator i = iconv_reg.begin(); i != iconv_reg.end(); i++)
  {
    delete i->first.first;
    delete i->first.second;
    iconv_close(i->second);
  }
  iconv_reg.clear();
  #ifdef SMSC_DEBUG
  iconv_instanceCounter = 0;
  smsc_log_debug(Logger::getInstance("u.x.Iconv"), "iconv registry cleared");
  #endif //SMSC_DEBUG
}


}
}
}
