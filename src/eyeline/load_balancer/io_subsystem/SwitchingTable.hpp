#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHINGTABLE_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_SWITCHINGTABLE_HPP__

# include <map>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class SwitchingTable : public utilx::Singleton<SwitchingTable> {
public:
  void insertSwitching(const LinkId& sme_link_id,
                       const LinkId& smsc_linkset_id);

  bool getSwitching(const LinkId& src_link_id,
                    LinkId* dst_link_id,
                    bool* is_src_link_incoming = NULL) const;

  /*
   * return true if link to sme with specified id exists and remove this switch entry.
   */
  bool removeSwitching(const LinkId& sme_link_id,
                       LinkId* smsc_linkset_id = NULL);

  void setSpecificSwitching(const LinkId& linkset_id,
                            const LinkId& specific_link_id,
                            unsigned color);

  // return empty linkid if there isn't switching entry
  LinkId getSpecificSwitching(const LinkId& linkset_id,
                              unsigned color) const;

  void removeSpecificSwitching(const LinkId& linkset_id,
                               unsigned color);
private:
  SwitchingTable()
  : _logger(smsc::logger::Logger::getInstance("io_subsystem"))
  {}
  friend class utilx::Singleton<SwitchingTable>;

  smsc::logger::Logger* _logger;
  typedef std::map<LinkId, LinkId> switch_table_t;
  switch_table_t _smeToSmcLinkset;
  switch_table_t _smscLinksetToSme;
  mutable smsc::core::synchronization::Mutex _switchTableLock;

  struct specific_switch_key {
    specific_switch_key(const LinkId& linkset_id, unsigned color)
    : _linksetId(linkset_id), _color(color)
    {}
    bool operator< (const specific_switch_key& rhs) const {
      if ( _linksetId != rhs._linksetId )
        return _linksetId < rhs._linksetId;
      if ( _color != rhs._color )
        return _color < rhs._color;

      return false;
    }
    LinkId _linksetId;
    unsigned _color;
  };
  typedef std::map<specific_switch_key, LinkId> specific_switch_table_t;
  specific_switch_table_t _specificSwitchTable;
  mutable smsc::core::synchronization::Mutex _specificSwitchTableLock;
};

}}}

#endif
