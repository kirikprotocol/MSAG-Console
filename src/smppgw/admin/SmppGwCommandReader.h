// 
// File:   SmppGwCommandReader.h
// Author: igork
//
// Created on 27 Май 2004 г., 16:41
//

#ifndef _SMPPGW_ADMIN_SmppGwCommandReader_H
#define	_SMPPGW_ADMIN_SmppGwCommandReader_H

#include "admin/protocol/Command.h"
#include "admin/protocol/CommandReader.h"
#include "core/network/Socket.hpp"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::core::network;

class SmppGwCommandReader : public CommandReader
{
public:
  SmppGwCommandReader(Socket * admSocket);
  virtual ~SmppGwCommandReader();
  
protected:
  virtual int getCommandIdByName(const char * const command_name);
  virtual Command * createCommand(int id, const DOMDocument *data);
  Hash<int> commandlist;  
};

}
}
}

#endif	/* _SMPPGW_ADMIN_SmppGwCommandReader_H */

