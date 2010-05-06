#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINKID_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_LINKID_HPP__

# include <netinet/in.h>
# include <string>

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class LinkId {
public:
  explicit LinkId(const std::string& link_id_value="")
  : _linkIdStr(link_id_value)
  {}

  std::string toString() const { return _linkIdStr; }

  bool operator==(const LinkId& rhs) const {
    return _linkIdStr == rhs._linkIdStr;
  }
  bool operator!=(const LinkId& rhs) const {
    return _linkIdStr != rhs._linkIdStr;
  }
  bool operator<(const LinkId& rhs) const {
    return _linkIdStr < rhs._linkIdStr;
  }
private:
  std::string _linkIdStr;
};

}}}

#endif

