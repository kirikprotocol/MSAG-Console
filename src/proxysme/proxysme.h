//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#ifdef _MSC_VER
#pragma warning(disable:4355)//'this' : used in base member initializer list
#endif

#if !defined SMSC_PROXYSME_MAIN_HEADER
#define SMSC_PROXYSME_MAIN_HEADER

#define SMSC_SMEPROXY_BEGIN namespace smsc { namespace proxysme {
#define SMSC_SMEPROXY_END }}
#define SMSC_SMEPROXY_SPACE smsc::proxysme

namespace smsc {
  namespace proxysme {
    enum {
    DEFAULTDCS_SMSC7BIT=0,
    DEFAULTDCS_LATIN1=3
    };
    // definition
  }
}


#include "../logger/Logger.h"
#include "../sme/SmppBase.hpp"

using smsc::sme::SmeConfig;

struct ProxyConfig {
  SmeConfig left,right;
  int quelimit;
  unsigned left_dcs,right_dcs;
  std::string admin_host;
  unsigned admin_port;
};

extern void MicroSleep();
extern void MacroSleep();

#endif
