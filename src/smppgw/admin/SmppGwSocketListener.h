// 
// File:   SmppGwSocketListener.h
// Author: igork
//
// Created on 26 Май 2004 г., 19:31
//

#ifndef _SMPPGW_ADMIN_SmppGwSocketListener_H
#define	_SMPPGW_ADMIN_SmppGwSocketListener_H

#include <admin/util/SocketListener.h>
#include <smppgw/admin/SmppGwCommandDispatcher.h>

namespace smsc {
namespace smppgw {
namespace admin {

using smsc::admin::util::SocketListener;

typedef SocketListener<SmppGwCommandDispatcher> SmppGwSocketListener;

}
}
}

#endif	/* _SMPPGW_ADMIN_SmppGwSocketListener_H */

