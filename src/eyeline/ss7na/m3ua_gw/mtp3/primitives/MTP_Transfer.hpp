#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPTRANSFER_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPTRANSFER_HPP__

# include <sys/types.h>
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace primitives {

class MTP_Transfer {
protected:
  MTP_Transfer(common::point_code_t opc, common::point_code_t dpc,
               uint8_t sls, uint8_t si, uint8_t ni, uint8_t msg_prio,
               utilx::variable_data_t data)
  : _opc(opc), _dpc(dpc), _sls(sls), _si(si), _ni(ni), _msgPrio(msg_prio), _data(data)
  {}

public:
  common::point_code_t getOPC() const {
    return _opc;
  }
  common::point_code_t getDPC() const {
    return _dpc;
  }
  uint8_t getSLS() const {
    return _sls;
  }
  uint8_t getServiceIndicator() const {
    return _si;
  }
  uint8_t getNetworkIndicator() const {
    return _ni;
  }
  uint8_t getMsgPrio() const {
    return _msgPrio;
  }
  utilx::variable_data_t getData() const {
    return _data;
  }

  std::string toString() const {
    char strBuf[1024];
    char hexdmpBuf[1024];
    snprintf(strBuf, sizeof(strBuf), "OPC=%u,DPC=%u,sls=%u,si=%u,ni=%u,prio=%u,data=%s",
             _opc, _dpc, _sls, _si, _ni, _msgPrio, utilx::hexdmp(hexdmpBuf, sizeof(hexdmpBuf), _data.data, _data.dataLen));
    return strBuf;
  }

private:
  common::point_code_t _opc, _dpc;
  uint8_t _sls, _si, _ni, _msgPrio;
  utilx::variable_data_t _data;
};

class MTP_Transfer_Req : public MTP_Transfer {
public:
  MTP_Transfer_Req(common::point_code_t opc, common::point_code_t dpc,
                   uint8_t sls, uint8_t si, uint8_t ni, uint8_t msg_prio,
                   utilx::variable_data_t data)
  : MTP_Transfer(opc, dpc, sls, si, ni, msg_prio, data)
  {}
};

class MTP_Transfer_Ind : public MTP_Transfer {
public:
  MTP_Transfer_Ind(common::point_code_t opc, common::point_code_t dpc,
                   uint8_t sls, uint8_t si, uint8_t ni, uint8_t msg_prio,
                   utilx::variable_data_t data)
  : MTP_Transfer(opc, dpc, sls, si, ni, msg_prio, data)
  {}
};

}}}}}

#endif
