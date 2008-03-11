#pragma ident "$Id$"
/* ************************************************************************* *
 * 
 * ************************************************************************* */
#ifndef __INMAN_TCPSERVER_DEFS_HPP
#define __INMAN_TCPSERVER_DEFS_HPP

#include "inman/services/tcpsrv/ConnManager.hpp"
using smsc::inman::interaction::INPCommandSetAC;

namespace smsc {
namespace inman {
namespace tcpsrv {

class ConnServiceITF {
public:
    virtual ConnectManagerAC *
        getConnManager(uint32_t sess_id, Connect * use_conn) = 0;
    virtual void
        rlseConnManager(uint32_t sess_id) = 0;
};

class TCPServerITF {
public:
    virtual bool registerProtocol(const INPCommandSetAC * cmd_set, ConnServiceITF * conn_srv) = 0;
};

} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_TCPSERVER_DEFS_HPP */

