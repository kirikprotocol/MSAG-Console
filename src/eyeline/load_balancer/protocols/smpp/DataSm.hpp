#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASM_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASM_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/DataSmResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class DataSm : public SMPPMessage {
public:
  DataSm();

  virtual size_t serialize(io_subsystem::Packet* packet) const;
  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual std::string toString() const;

  uint16_t getSarMsgRefNum() const;
  bool isSetSarMsgRefNum() const { return _isSetSarMsgRefNum; }

  uint8_t getSarTotalSegments() const;
  bool isSetSarTotalSegments() const { return _isSetSarTotalSegments; }

  uint8_t getSarSegmentSeqNum() const;
  bool isSetSarSegmentSeqNum() const { return _isSetSarSegmentSeqNum; }

  DataSmResp* makeResponse(uint32_t command_status) const;

  static const uint32_t _MSGCODE = 0x00000103;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

private:
  const uint8_t* _rawMessageBody;
  uint32_t _rawMessageBodyLen;
  uint16_t _sarMsgRefNum;
  uint8_t _sarTotalSegments, _sarSegmentSeqNum;
  bool _isSetSarMsgRefNum, _isSetSarTotalSegments, _isSetSarSegmentSeqNum;
};

}}}}

#endif

