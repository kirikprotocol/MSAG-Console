#ifndef __BDB_AGENT_APPLICATIONPACKETS_HPP__
# define __BDB_AGENT_APPLICATIONPACKETS_HPP__ 1

# include <sys/types.h>
# include <vector>
# include <string>
# include <memory>
# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>
# include <string.h>
# include <util/BufferSerialization.hpp>
# include <util/comm_comp/ReqAppPacket_Subscriber.hpp>
# include <util/comm_comp/TEvent.hpp>
# include <util/comm_comp/SerializationIface.hpp>

namespace mmbox {
namespace app_protocol {

/*
** This class represents response object being transmitted from db server to client.
**
** General application object's format is:
** |--- 2 bytes ---|  |-- data of variable length --|
** ! object's code !  !       object's data         !
**
** Lenght of application data (2 bytes for object's code + size of object's data) specified in 
** header of transport packet and isn't specified in application's classes.
*/
class ResponseApplicationPacket : public smsc::util::comm_comp::SerializerIface {
public:
  typedef enum { STATUS_OK = 0, NO_DATA_FOUND=100,
                 CONGESTION_CONDITION=254, BDB_FAILURE = 255, GENERIC_FAILURE = -1 } result_status_t;
  typedef uint16_t packet_code_t;

  ResponseApplicationPacket(packet_code_t pck_code, result_status_t status);

  virtual ~ResponseApplicationPacket() {}
  virtual void serialize(std::vector<uint8_t>& objectBuffer) const;
  virtual std::string toString() const;

  result_status_t getStatus() const;
private:
  enum { RES_APP_PACKET_CODE_MASK = 0x8000 };
  packet_code_t _packet_code;
  uint32_t _status;
};

/*
** This class represents request object being transmitted from client to db server.
*/
class RequestApplicationPacket : public smsc::util::comm_comp::DeserializerIface {
public:
  virtual ~RequestApplicationPacket() {}
  virtual std::string toString() const = 0;
  typedef uint16_t packet_code_t;
  // get appliction object's code
  virtual packet_code_t getPacketCode() const = 0;
  // create object from raw data getting from network
  virtual void deserialize(const std::vector<uint8_t>& objectBuffer);
  // create event from object.
  virtual const smsc::util::comm_comp::AbstractEvent* createEvent() = 0;
  // create response object corresponding to request object
  virtual ResponseApplicationPacket* createAppResponse(ResponseApplicationPacket::result_status_t status) const = 0;
  // utility method - get object's code from raw data
  static packet_code_t extractPacketCode(const std::vector<uint8_t>& buf);
protected:
  smsc::util::SerializationBuffer _deserializerBuf;
};

}}

#endif
