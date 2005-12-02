#ifndef __COMMAND_GEN_H__
#define __COMMAND_GEN_H__

#define SMPP_COMMANDS_COUNT 9


void  String2SCAGCommang(scag::transport::smpp::SmppCommand &cmd, 
       std::string cmname,
       std::string str_oa,
       uint8_t oa_tn,
       uint8_t oa_np,
       std::string str_da,
       uint8_t da_tn,
       uint8_t da_np,
       uint32_t dialogid);


#endif