// 
// File:   SmppGwCommandReader.h
// Author: igork
//
// Created on 27 ��� 2004 �., 16:41
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
using smsc::core::buffers::Hash;




class SmppGwCommandReader : public CommandReader
{
public:
  SmppGwCommandReader(Socket * admSocket);
  virtual ~SmppGwCommandReader();
  
protected:


  typedef Hash<int> _CommandList;
  _CommandList commandlist;


  virtual int getCommandIdByName(const char * const command_name);
  virtual Command * createCommand(int id, const DOMDocument *data);
  
};

}
}
}

#endif	/* _SMPPGW_ADMIN_SmppGwCommandReader_H */

