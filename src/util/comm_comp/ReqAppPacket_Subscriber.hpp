#ifndef __BDB_AGENT_REQAPPPACKET_SUBSCRIBER__
# define __BDB_AGENT_REQAPPPACKET_SUBSCRIBER__ 1

# include <string>

namespace smsc {
namespace util {
namespace comm_comp {

/*
** Container for object is being put to Objects Queue. Container contains pointer to real object
** and connection id from which object's data was read
*/
template <class OBJ>
struct AcceptedObjInfo {
  AcceptedObjInfo(const OBJ* req=NULL, const std::string& connId="")
    : reqAppObj(req), connectId(connId) {}
  const OBJ* reqAppObj;
  std::string connectId;
};

}}}

#endif
