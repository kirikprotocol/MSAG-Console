static char const ident[] = "$Id$";
#include "UpdateLocation.hpp"
#include "mtsmsme/processor/util.hpp"
//#include "mtsmsme/processor/ACRepo.hpp"
using std::vector;
using smsc::mtsmsme::comp::UpdateLocationMessage;
using smsc::mtsmsme::processor::util::dump;
using smsc::mtsmsme::processor::TrId;
int main(int argc, char* argv[])
{
  UpdateLocationMessage msg;
  TrId id; id.size=4; id.buf[0] = 0x29; id.buf[1] = 0x00; id.buf[2] = 0x01; id.buf[3] = 0xB3;
  msg.setOTID(id);
  msg.setComponent("250130124323100","1979139860001","1979139860001");
  vector<unsigned char> ulmsg;
  msg.encode(ulmsg);
  printf("UpdateLocation[%d]={%s}",ulmsg.size(),dump((uint16_t)ulmsg.size(),&ulmsg[0]).c_str());
  //smsc::mtsmsme::processor::SuperTestFunction();
}
