#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_ENCODE_HPP__
#define __SMSC_MTSMSME_PROCESSOR_ENCODE_HPP__

#include <vector>
#include "mtsmsme/processor/Message.hpp"
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace encode{

using std::vector;
using smsc::logger::Logger;

class Encoder {
  public:
    int encode_mt_resp(ContMsg& msg,vector<unsigned char>& buf);
    int encode_resp(ContMsg& msg,vector<unsigned char>& buf);
    int encode_resp(EndMsg& msg,vector<unsigned char>& buf);
    int encode_mt_resp(EndMsg& msg,vector<unsigned char>& buf);
    int encodeResourceLimitation(TrId dtid,vector<unsigned char>& buf);
    int encodeBadTrPortion(TrId dtid,vector<unsigned char>& buf);
    int encodeACNotSupported(TrId dtid,AC& ac,vector<unsigned char>& buf);
    Encoder();
    ~Encoder();
};

}//namespace encode
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
#endif
