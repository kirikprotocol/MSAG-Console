// 
// File:   SCAGSocketListener.h
// Author: igork
//
// Created on 26 ��� 2004 �., 19:31
//

#ifndef _SCAG_ADMIN_SCAGSocketListener2_H
#define	_SCAG_ADMIN_SCAGSocketListener2_H

#include <admin/util/SocketListener.h>
#include <scag/admin/SCAGCommandDispatcher2.h>

namespace scag2 {
namespace admin {

using smsc::admin::util::SocketListener;

typedef SocketListener<SCAGCommandDispatcher> SCAGSocketListener;

}
}

#endif	/* _SCAG_ADMIN_SmppGwSocketListener_H */

