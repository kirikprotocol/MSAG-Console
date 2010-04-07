#ifndef __EYELINE_SS7NA_M3UAGW_POINTSTATUSMGMT_POINTSTATUSTABLE_HPP__
# define __EYELINE_SS7NA_M3UAGW_POINTSTATUSMGMT_POINTSTATUSTABLE_HPP__

# include "core/buffers/IntHash.hpp"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/ss7na/common/PointCode.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace point_status_mgmt {

enum point_status_e {
  POINT_ALLOWED, POINT_PROHIBITED, POINT_RESRICTED, UNKNOW_POINT
};

class PointStatusTable : public utilx::Singleton<PointStatusTable> {
public:
  point_status_e checkStatus(common::point_code_t pc) const {
    smsc::core::synchronization::MutexGuard synchroniz(_lock);

    point_status_e pointStatus = UNKNOW_POINT;
    _pcStatusTable.Get(pc, pointStatus);
    return pointStatus;
  }

  void updateStatus(common::point_code_t pc, point_status_e new_status) {
    smsc::core::synchronization::MutexGuard synchroniz(_lock);

    smsc_log_info(_logger, "PointStatusTable::updateStatus::: set status=%s for point=%u",
                   toString(new_status), pc);
    point_status_e* pointStatus = _pcStatusTable.GetPtr(pc);
    if ( pointStatus )
      *pointStatus = new_status;
    else
      _pcStatusTable.Insert(pc, new_status);
  }

private:
  PointStatusTable()
  : _logger(smsc::logger::Logger::getInstance("mtp3"))
  {}
  friend class utilx::Singleton<PointStatusTable>;

  const char* toString(point_status_e new_status) const {
    if (new_status == POINT_ALLOWED)
      return "POINT_ALLOWED";

    if (new_status == POINT_PROHIBITED)
      return "POINT_PROHIBITED";

    return "POINT_RESRICTED, UNKNOW_POINT";
  }
  mutable smsc::core::synchronization::Mutex _lock;
  smsc::core::buffers::IntHash<point_status_e> _pcStatusTable;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
