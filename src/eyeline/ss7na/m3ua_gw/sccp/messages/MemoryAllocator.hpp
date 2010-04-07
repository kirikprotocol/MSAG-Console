#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_MEMORYALLOCATOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_MEMORYALLOCATOR_HPP_

# include "eyeline/utilx/PreallocatedMemoryManager.hpp"
# include "eyeline/utilx/maxArrayElement.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/UDTS.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDT.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/XUDTS.hpp"

namespace eyeline {
namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_SCCP_MESSAGE>() {
  static size_t msgsSzArr[]={sizeof(ss7na::m3ua_gw::sccp::messages::UDT),
                             sizeof(ss7na::m3ua_gw::sccp::messages::UDTS),
                             sizeof(ss7na::m3ua_gw::sccp::messages::XUDT),
                             sizeof(ss7na::m3ua_gw::sccp::messages::XUDTS)};
  static const size_t MAX_SCCPMESSAGE_OBJECT_SIZE=maxArrayElement(msgsSzArr);
  static TSD_Init_Helper<MEM_FOR_SCCP_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(MAX_SCCPMESSAGE_OBJECT_SIZE, tsd_init._tsd_memory_key);

  return res;
}

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_RESPONSE_SCCP_MESSAGE>() {
  static size_t msgsSzArr[]={sizeof(ss7na::m3ua_gw::sccp::messages::UDTS),
                             sizeof(ss7na::m3ua_gw::sccp::messages::XUDTS)};
  static const size_t MAX_SCCPMESSAGE_OBJECT_SIZE=maxArrayElement(msgsSzArr);
  static TSD_Init_Helper<MEM_FOR_RESPONSE_SCCP_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(MAX_SCCPMESSAGE_OBJECT_SIZE, tsd_init._tsd_memory_key);

  return res;
}

}}

#endif
