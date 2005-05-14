// 
// File:   SCAGCommandReader.h
// Author: igork
//
// Created on 27 Май 2004 г., 16:41
//

#ifndef _SCAG_ADMIN_SCAGCommandReader_H
#define	_SCAG_ADMIN_SCAGCommandReader_H

#include "admin/protocol/Command.h"
#include "admin/protocol/CommandReader.h"
#include "core/network/Socket.hpp"

namespace smsc {
namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::core::network;

class SCAGCommandReader : public CommandReader
{
public:
  SCAGCommandReader(Socket * admSocket);
  virtual ~SCAGCommandReader();
  
protected:
  virtual int getCommandIdByName(const char * const command_name);
  virtual Command * createCommand(int id, const DOMDocument *data);
  Hash<int> commandlist;  
};

}
}
}

#endif	/* _SCAG_ADMIN_SCAGCommandReader_H */

