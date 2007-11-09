#ifndef __SMSC_INMAN_USS_USSBALANCECONNECT_HPP__
# define __SMSC_INMAN_USS_USSBALANCECONNECT_HPP__ 1

# include <inman/interaction/connect.hpp>
# include <inman/interaction/serializer.hpp>
# include <memory>
# include <logger/Logger.h>
# include "UssServiceCfg.hpp"

namespace smsc {
namespace inman {
namespace uss {

//##ModelId=457534DE0050
class USSBalanceConnect : public smsc::inman::interaction::ConnectListenerITF {
public:
  USSBalanceConnect(smsc::logger::Logger* logger, const UssService_CFG& cfg);
  ~USSBalanceConnect();
  //##ModelId=4575350D008E
  void onPacketReceived(smsc::inman::interaction::Connect* conn,
                         std::auto_ptr<smsc::inman::interaction::SerializablePacketAC>& recv_cmd)
    throw(std::exception);

  //##ModelId=45753514006F
  void onConnectError(smsc::inman::interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc);
private:
  smsc::logger::Logger *_logger;
  const UssService_CFG& _cfg;

  class USSProcSearchCrit {
  public:
    USSProcSearchCrit(unsigned char ssn,
                      const TonNpiAddress& addr,
                      const smsc::inman::interaction::Connect* conn)
      : _ssn(ssn), _addr(addr), _conn(conn) {}

    bool operator<(const USSProcSearchCrit& rhs) const {
      if ( _ssn < rhs._ssn ||
           _addr.toString() < rhs._addr.toString() ||
           _conn < rhs._conn ) return true;
      else return false;
    }
  private:
    unsigned char _ssn;
    TonNpiAddress _addr;
    const smsc::inman::interaction::Connect* _conn;
  };

  typedef std::vector<USSProcSearchCrit>  CreatedSearchCritList_t;
  CreatedSearchCritList_t _searchCritForCreatedReqProcessors;
};


}
}
}

#endif
