#ifndef __EYELINE_SS7NA_SUAGW_IODISPATCHER_CONNECTMGR_HPP__
# define __EYELINE_SS7NA_SUAGW_IODISPATCHER_CONNECTMGR_HPP__

# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectionClosedEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace io_dispatcher {

typedef common::io_dispatcher::ConnectMgrTmpl<ConnectionClosedEvent> ConnectMgr;

}}}}

#endif
