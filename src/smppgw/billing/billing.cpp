#include "bill.hpp"
#include <exception>
#include <string>
#include <stdexcept>
#include <dlfcn.h>
#include <unistd.h>

namespace smsc{
namespace smppgw{
namespace billing{

typedef void* (*getBillingInterfaceFn)();

static void* dlHandle=0;
static IBillingInterface* billingIface=0;

void InitBillingInterface(const char* billingModule)
{
  dlHandle = dlopen(billingModule, RTLD_LAZY);
  if(!dlHandle)throw std::runtime_error("Failed to load billing module");
  getBillingInterfaceFn fn= (getBillingInterfaceFn)dlsym(dlHandle, "getBillingInterface");
  if(!fn)throw std::runtime_error("Invalid billing module");
  billingIface=(IBillingInterface*)fn();
}

IBillingInterface* GetBillingInterface()
{
  return billingIface;
}


}//billing
}//smppgw
}//smsc
